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
  PmidAccountHolderService(const passport::Pmid& pmid, routing::Routing& routing, Db& db);
  template<typename Data>
  void HandleMessage(const nfs::Message& message, const routing::ReplyFunctor& reply_functor);
  void HandleMessage(const nfs::Message& message, const routing::ReplyFunctor& reply_functor);
  void HandleChurnEvent(routing::MatrixChange matrix_change);

 private:
  PmidAccountHolderService(const PmidAccountHolderService&);
  PmidAccountHolderService& operator=(const PmidAccountHolderService&);
  PmidAccountHolderService(PmidAccountHolderService&&);
  PmidAccountHolderService& operator=(PmidAccountHolderService&&);

  void ValidateDataSender(const nfs::Message& message) const;
  void ValidateGenericSender(const nfs::Message& message) const;

  void CreatePmidAccount(const nfs::Message& message);
  void GetPmidTotals(const nfs::Message& message);

  // =============== Put/Delete data ================================================================
  template<typename Data>
  void HandlePut(const nfs::Message& message, const routing::ReplyFunctor& reply_functor);
  template<typename Data>
  void HandleDelete(const nfs::Message& message, const routing::ReplyFunctor& reply_functor);
  template<typename Data>
  void HandlePutResult(const nfs::Reply& overall_result,
                       const nfs::Message& message,
                       routing::ReplyFunctor reply_functor);

  void SendReplyAndAddToAccumulator(const nfs::Message& message,
                                    const routing::ReplyFunctor& reply_functor,
                                    const nfs::Reply& reply);

  // =============== Sync ==========================================================================
  void Sync(const PmidName& account_name);
  void HandleSync(const nfs::Message& message);

  // =============== Account transfer ==============================================================
  void TransferAccount(const PmidName& account_name, const NodeId& new_node);
  void HandleAccountTransfer(const nfs::Message& message);

  void ValidateMessage(const nfs::Message& message) const;

  template<typename Data, nfs::MessageAction action>
  void AddLocalUnresolvedEntryThenSync(const nfs::Message& message);
  template<typename Data, nfs::MessageAction action>
  void ReplyToMetadataManagers(const std::vector<PmidAccountResolvedEntry>& resolved_entries,
                               const PmidName& pmid_name);

  routing::Routing& routing_;
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
