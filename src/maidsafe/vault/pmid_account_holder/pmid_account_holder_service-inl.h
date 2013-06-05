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

#ifndef MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_SERVICE_INL_H_
#define MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_SERVICE_INL_H_

#include <exception>
#include <string>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/vault/utils.h"
#include "maidsafe/vault/unresolved_element.h"

namespace maidsafe {
namespace vault {

namespace detail {

template<typename Data, nfs::MessageAction Action>
PmidAccountUnresolvedEntry CreateUnresolvedEntry(const nfs::Message& message,
                                                 const NodeId& this_id) {
  static_assert(Action == nfs::MessageAction::kPut || Action == nfs::MessageAction::kDelete,
                "Action must be either kPut of kDelete.");
  return PmidAccountUnresolvedEntry(
            std::make_pair(GetDataNameVariant(DataTagValue(message.data().type.get()),
                                              Identity(message.data().name)),
                           Action),
            message.data().content.string().size(),
            this_id);
}

PmidName GetPmidAccountName(const nfs::Message& message);

}  // namespace detail

template<typename Data>
void PmidAccountHolderService::HandleMessage(const nfs::Message& message,
                                             const routing::ReplyFunctor& reply_functor) {
  ValidateDataSender(message);
  nfs::Reply reply(CommonErrors::success);
  {
    std::lock_guard<std::mutex> lock(accumulator_mutex_);
    if (accumulator_.CheckHandled(message, reply))
      return reply_functor(reply.Serialise()->string());
  }

  if (message.data().action == nfs::MessageAction::kPut) {
    HandlePut<Data>(message, reply_functor);
  } else if (message.data().action == nfs::MessageAction::kDelete) {
    HandleDelete<Data>(message, reply_functor);
  } else {
    reply = nfs::Reply(VaultErrors::operation_not_supported, message.Serialise().data);
    SendReplyAndAddToAccumulator(message, reply_functor, reply);
  }
}

template<typename Data>
void PmidAccountHolderService::HandlePut(const nfs::Message& message,
                                         const routing::ReplyFunctor& reply_functor) {
  maidsafe_error return_code(CommonErrors::success);
  try {
    Data data(typename Data::name_type(message.data().name),
              typename Data::serialised_type(message.data().content));
    
    auto put_op(std::make_shared<nfs::OperationOp>(
        kPutRepliesSuccessesRequired_,
        [this, message, reply_functor](nfs::Reply overall_result) {
            this->HandlePutResult<Data>(overall_result, message, reply_functor);
        }));

    nfs_.Put(PmidName(detail::GetPmidAccountName(message)),
             data,
             [put_op](std::string serialised_reply) {
                 nfs::HandleOperationReply(put_op, serialised_reply);
             });
    return;
  }
  catch(const maidsafe_error& error) {
    LOG(kWarning) << error.what();
    return_code = error;
  }
  catch(...) {
    LOG(kWarning) << "Unknown error.";
    return_code = MakeError(CommonErrors::unknown);
  }
  nfs::Reply reply(return_code, message.Serialise().data);
  SendReplyAndAddToAccumulator(message, reply_functor, reply);
}

template<typename Data>
void PmidAccountHolderService::HandleDelete(const nfs::Message& message,
                                            const routing::ReplyFunctor& reply_functor) {
  SendReplyAndAddToAccumulator(message, reply_functor, nfs::Reply(CommonErrors::success));
  try {
    auto account_name(detail::GetPmidAccountName(message));
    typename Data::name_type data_name(message.data().name);
    pmid_account_handler_.Delete<Data>(account_name, data_name);
    AddLocalUnresolvedEntryThenSync<Data, nfs::MessageAction::kDelete>(message);
    nfs_.Delete<Data>(message.data_holder(), data_name, [](std::string) {});
  }
  catch(const maidsafe_error& error) {
    LOG(kWarning) << error.what();
  }
  catch(...) {
    LOG(kWarning) << "Unknown error.";
  }
}

template<typename Data>
void PmidAccountHolderService::HandlePutResult(const nfs::Reply& overall_result,
                                               const nfs::Message& message,
                                               routing::ReplyFunctor reply_functor) {
  if (overall_result.IsSuccess()) {
    nfs::Reply reply(CommonErrors::success);
    AddLocalUnresolvedEntryThenSync<Data, nfs::MessageAction::kPut>(message);
    SendReplyAndAddToAccumulator(message, reply_functor, reply);
  } else {
    SendReplyAndAddToAccumulator(message, reply_functor, overall_result);
  }
}

template<typename Data, nfs::MessageAction Action>
void PmidAccountHolderService::AddLocalUnresolvedEntryThenSync(const nfs::Message& message) {
  auto account_name(detail::GetPmidAccountName(message));
  auto unresolved_entry(detail::CreateUnresolvedEntry<Data, Action>(message, routing_.kNodeId()));
  pmid_account_handler_.AddLocalUnresolvedEntry(account_name, unresolved_entry);
  Sync(account_name);
}

template<typename Data, nfs::MessageAction Action>
void PmidAccountHolderService::ReplyToMetadataManagers(
      const std::vector<PmidAccountResolvedEntry>& resolved_entries,
      const PmidName& pmid_name) {
  /*for (auto& resolved_entry : resolved_entries) {
    
    
  }*/
}

}  // namespace vault
}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_SERVICE_INL_H_
