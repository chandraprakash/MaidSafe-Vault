/***************************************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#include "maidsafe/vault/pmid_account_holder/pmid_account_holder_service.h"

#include "maidsafe/common/error.h"
#include "maidsafe/nfs/generic_message.h"
#include "maidsafe/nfs/pmid_health.h"
#include "maidsafe/vault/pmid_account_holder/pmid_account_pb.h"


namespace fs = boost::filesystem;

namespace maidsafe {

namespace vault {

const int PmidAccountHolderService::kPutRequestsRequired_(3);
const int PmidAccountHolderService::kDeleteRequestsRequired_(3);

PmidAccountHolderService::PmidAccountHolderService(const passport::Pmid& pmid,
                                                   routing::Routing& routing,
                                                   const boost::filesystem::path& vault_root_dir)
    : routing_(routing),
      accumulator_mutex_(),
      accumulator_(),
      pmid_account_handler_(vault_root_dir),
      nfs_(routing, pmid) {}

void PmidAccountHolderService::HandleGenericMessage(const nfs::GenericMessage& generic_message,
                                                    const routing::ReplyFunctor& reply_functor) {
// HandleNewComer(p_maid);
  nfs::GenericMessage::Action action(generic_message.action());
  switch (action) {
    case nfs::GenericMessage::Action::kGetPmidHealth:
      return HandlePmidHealth(generic_message, reply_functor);
//    case nfs::GenericMessage::Action::kSynchronise:
//      return HandleSyncMessage(generic_message, reply_functor);
    default:
      LOG(kError) << "Unhandled Post action type";
  }
}
void PmidAccountHolderService::HandlePmidHealth(const nfs::GenericMessage& generic_message,
                                                const routing::ReplyFunctor& reply_functor) {
  try {
    if (generic_message.source().persona != nfs::Persona::kMaidAccountHolder ||
        generic_message.destination_persona() != nfs::Persona::kPmidAccountHolder)
      ThrowError(CommonErrors::invalid_parameter);

    PmidAccountHandler::AccountHealth health;
    PmidName name(Identity(generic_message.name()));
    health = pmid_account_handler_.GetAccountHealth(name);
    nfs::PmidHealth pmid_health(name);
    pmid_health.health = health.health;
    pmid_health.offered_space = health.offered_space;
    pmid_health.online = routing_.IsConnectedVault(NodeId(name->string()));
    nfs::Reply reply(CommonErrors::success, pmid_health.Serialise());
    reply_functor(reply.Serialise()->string());
  }
  catch(const maidsafe_error& error) {
    nfs::Reply reply(error, generic_message.Serialise());
    reply_functor(reply.Serialise()->string());
  }
  catch(...) {
    nfs::Reply reply(CommonErrors::unknown, generic_message.Serialise());
    reply_functor(reply.Serialise()->string());
  }
}

void PmidAccountHolderService::TriggerSync(
    /*const std::vector<routing::NodeInfo>& new_close_nodes*/) {
  // Operations to be done when we this call is received
  CheckAccounts();
}

void PmidAccountHolderService::CheckAccounts() {
  // Non-archived
  std::vector<PmidName> accounts_held(pmid_account_handler_.GetAccountNames());
  for (auto& account_name : accounts_held)
    IsResponsibleFor(account_name);

  // Archived
  pmid_account_handler_.PruneArchivedAccounts(
      [this] (const PmidName& pmid_name) {
        return routing::GroupRangeStatus::kOutwithRange ==
               routing_.IsNodeIdInGroupRange(NodeId(pmid_name));
      });
}

void PmidAccountHolderService::CheckResponsibility(const PmidName& account_name) {
  bool is_connected(routing_.IsConnectedVault(NodeId(account_name)));
  bool marked_as_online(pmid_account_handler_.AccountStatus(account_name));
  switch (routing_.IsNodeIdInGroupRange(NodeId(account_name))) {
    case routing::kOutwithRange:
    case routing::kInProximalRange:
      pmid_account_handler_.MoveAccountToArchive(account_name);
      break;
    case routing::kInRange:
      if (marked_as_online && !is_connected) {
        InformOfDataHolderDown(account_name);
      } else if (!marked_as_online && is_connected) {
        InformOfDataHolderUp(account_name);
      }
      break;
    default:
      break;
  }
}

void PmidAccountHolderService::ValidateSender(const nfs::DataMessage& data_message) const {
  if (!data_message.HasDataHolder())
    ThrowError(VaultErrors::permission_denied);

  if (!routing_.IsConnectedVault(NodeId(data_message.data_holder()->string())))
    ThrowError(VaultErrors::permission_denied);

  if (routing_.EstimateInGroup(data_message.source().node_id,
                               NodeId(data_message.data().name)))
    ThrowError(VaultErrors::permission_denied);

  if (data_message.source().persona != nfs::Persona::kMetadataManager ||
      data_message.destination_persona() != nfs::Persona::kPmidAccountHolder)
    ThrowError(CommonErrors::invalid_parameter);
}

void PmidAccountHolderService::InformOfDataHolderDown(const PmidName& pmid_name) {
  InformAboutDataHolder(pmid_name, false);
  pmid_account_handler_.SetDataHolderDown(pmid_name);
}

void PmidAccountHolderService::InformOfDataHolderUp(const PmidName& pmid_name) {
  InformAboutDataHolder(pmid_name, true);
  pmid_account_handler_.SetDataHolderUp(pmid_name);
}

void PmidAccountHolderService::InformAboutDataHolder(const PmidName& pmid_name, bool node_up) {
  auto names(pmid_account_handler_.GetArchiveFileNames(pmid_name));
  for (auto ritr(names.rbegin()); ritr != names.rend(); ++ritr) {
    if (StatusHasReverted(pmid_name, node_up)) {
      RevertMessages(pmid_name, names.rbegin(), ritr, !node_up);
      return;
    }

    std::set<PmidName> metadata_manager_ids(GetDataNamesInFile(pmid_name, *ritr));
    SendMessages(pmid_name, metadata_manager_ids, node_up);
  }
}

std::set<PmidName> PmidAccountHolderService::GetDataNamesInFile(
    const PmidName& pmid_name,
    const boost::filesystem::path& path) const {
  // pare file serialse as string sed to has pmidah to send to mm
  NonEmptyString file_content(pmid_account_handler_.GetArchiveFile(pmid_name, path));
  protobuf::PmidRecord pmid_data;
  pmid_data.ParseFromString(file_content.string());
  std::set<PmidName> metadata_manager_ids;
  //for (int n(0); n != pmid_data.stored_total_size(); ++n)
  //  metadata_manager_ids.insert(PmidName(Identity(pmid_data.data_stored(n).name())));
  return metadata_manager_ids;
}


void PmidAccountHolderService::RevertMessages(const PmidName& pmid_name,
                                              const std::vector<fs::path>::reverse_iterator& begin,
                                              std::vector<fs::path>::reverse_iterator& current,
                                              bool node_up) {
  while (current != begin) {
    std::set<PmidName> metadata_manager_ids(GetDataNamesInFile(pmid_name, *current));
    SendMessages(pmid_name, metadata_manager_ids, node_up);
    --current;
  }

  node_up ?  pmid_account_handler_.SetDataHolderUp(pmid_name) :
             pmid_account_handler_.SetDataHolderDown(pmid_name);
}

void PmidAccountHolderService::SendMessages(const PmidName& /*pmid_name*/,
                                            const std::set<PmidName>& /*metadata_manager_ids*/,
                                            bool /*node_up*/) {
//  for (const PmidName& metadata_manager_id : metadata_manager_ids) {
    //  TODO(dirvine) implement
        //    nfs_.DataHolderStatusChanged(NodeId(metadata_manager_id), NodeId(pmid_name), node_up);
//  }
}

}  // namespace vault

}  // namespace maidsafe
