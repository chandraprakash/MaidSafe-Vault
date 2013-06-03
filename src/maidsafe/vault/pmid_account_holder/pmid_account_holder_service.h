/***************************************************************************************************
 *  Copyright 2012 PmidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of PmidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of PmidSafe.net.                                  *
 **************************************************************************************************/

#ifndef MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_SERVICE_H_
#define MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_SERVICE_H_

#include <mutex>
#include <set>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/rsa.h"
#include "maidsafe/passport/types.h"
#include "maidsafe/routing/routing_api.h"
#include "maidsafe/nfs/message.h"
#include "maidsafe/nfs/public_key_getter.h"
#include "maidsafe/nfs/reply.h"

#include "maidsafe/vault/accumulator.h"
#include "maidsafe/vault/pmid_account_holder/pmid_account_handler.h"
#include "maidsafe/vault/types.h"


namespace maidsafe {
namespace vault {

class PmidAccountHolderService {
 public:
  typedef nfs::Message Message;
  typedef nfs::Reply Reply;
  typedef routing::Routing Routing;
  typedef routing::ReplyFunctor ReplyFunctor;
  
  PmidAccountHolderService(const passport::Pmid& pmid, routing::Routing& routing, Db& db);
  template<typename Data>
  void HandleMessage(const Message& message, const ReplyFunctor& reply_functor);
  void HandleMessage(const Message& message, const ReplyFunctor& reply_functor);
  void HandleChurnEvent(routing::MatrixChange matrix_change);

 private:
  PmidAccountHolderService(const PmidAccountHolderService&);
  PmidAccountHolderService& operator=(const PmidAccountHolderService&);
  PmidAccountHolderService(PmidAccountHolderService&&);
  PmidAccountHolderService& operator=(PmidAccountHolderService&&);

  void ValidateDataSender(const Message& message) const;
  void ValidateGenericSender(const Message& message) const;

  void SendReplyAndAddToAccumulator(const Message& message,
                                    const ReplyFunctor& reply_functor,
                                    const Reply& reply);

  template<typename Data>
  void HandlePut(const Message& message, const ReplyFunctor& reply_functor);
  template<typename Data>
  void HandleDelete(const Message& message, const ReplyFunctor& reply_functor);

  void HandleGetPmidTotals(const Message& message, const ReplyFunctor& reply_functor);

  template<typename Data>
  void AdjustAccount(const Message& message);
  template<typename Data>
  void SendMessages(const Message& message);

  template<typename Data>
  void HandlePutResult(const Reply& overall_result,
                       const Message& message,
                       ReplyFunctor reply_functor);

  bool HandleReceivedSyncData(const NonEmptyString& serialised_account);


  // =============== Sync ==========================================================================
  void Sync(const PmidName& account_name);
  void HandleSync(const nfs::Message& message);

  // =============== Account transfer ==============================================================
  void TransferAccount(const PmidName& account_name, const NodeId& new_node);
  void HandleAccountTransfer(const nfs::Message& message);

  void ValidateMessage(const nfs::Message& message) const;
  void InformOfDataHolderDown(const PmidName& pmid_name);
  void InformOfDataHolderUp(const PmidName& pmid_name);
  void InformAboutDataHolder(const PmidName& pmid_name, bool node_up);

  bool StatusHasReverted(const PmidName& pmid_name, bool node_up) const;
  void RevertMessages(const PmidName& pmid_name,
                      const std::vector<boost::filesystem::path>::reverse_iterator& begin,
                      std::vector<boost::filesystem::path>::reverse_iterator& current,
                      bool node_up);
  std::set<PmidName> GetDataNamesInFile(const PmidName& pmid_name,
                                        const boost::filesystem::path& path) const;
  void SendMessages(const PmidName& pmid_name,
                    const std::set<PmidName>& metadata_manager_ids,
                    bool node_up);

  template<typename Data, nfs::MessageAction action>
  void AddLocalUnresolvedEntryThenSync(const nfs::Message& message);

  Routing& routing_;
  std::mutex accumulator_mutex_;
  Accumulator<PmidName> accumulator_;
  PmidAccountHandler pmid_account_handler_;
  PmidAccountHolderNfs nfs_;
  static const int kPutRepliesSuccessesRequired_;
  static const int kDeleteRequestsRequired_;
};

}  // namespace vault
}  // namespace maidsafe

#include "maidsafe/vault/pmid_account_holder/pmid_account_holder_service-inl.h"

#endif  // MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_SERVICE_H_
