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

#ifndef MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HANDLER_H_
#define MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HANDLER_H_

#include <cstdint>
#include <future>
#include <map>
#include <mutex>
#include <vector>

#include "boost/filesystem/path.hpp"


#include "maidsafe/vault/pmid_account_holder/pmid_account.h"
#include "maidsafe/vault/types.h"


namespace maidsafe {

namespace vault {

class PmidAccountHandler {
 public:
  typedef std::map<typename PmidAccount::name_type, std::unique_ptr<PmidAccount>> AccountMap;

  explicit PmidAccountHandler(Db& db, const NodeId& this_node_id);

  // Account operations
  void CreateAccount(const PmidName& account_name);
  bool ApplyAccountTransfer(const PmidName& account_name, const NodeId& source_id,
                            const PmidAccount::serialised_type& serialised_pmid_account_details);
  void AddAccount(std::unique_ptr<PmidAccount> pmid_account);
  void DeleteAccount(const PmidName& account_name);
  PmidAccount::DataHolderStatus AccountStatus(const PmidName& account_name) const;
  void SetDataHolderGoingDown(const PmidName& account_name);
  void SetDataHolderDown(const PmidName& account_name);
  void SetDataHolderGoingUp(const PmidName& account_name);
  void SetDataHolderUp(const PmidName& account_name);
  
  void AddLocalUnresolvedEntry(const PmidName& account_name,
                               const PmidAccountUnresolvedEntry& unresolved_entry);
  PmidRecord GetPmidRecord(const PmidName& account_name);

  // Sync operations
  std::vector<PmidName> GetAccountNames() const;
  std::vector<PmidName> GetArchivedAccountNames() const;
  PmidAccount::serialised_type GetSerialisedAccount(const PmidName& account_name) const;
  NonEmptyString GetSyncData(const PmidName& account_name);
  std::vector<PmidAccountResolvedEntry> ApplySyncData(const PmidName& account_name,
                     const NonEmptyString& serialised_unresolved_entries);
  void ReplaceNodeInSyncList(const PmidName& account_name,
                             const NodeId& old_node,
                             const NodeId& new_node);
  void IncrementSyncAttempts(const PmidName& account_name);

  // Data operations
  template<typename Data>
  void Put(const PmidName& account_name, const typename Data::name_type& data_name, int32_t size);
  template<typename Data>
  void Delete(const PmidName& account_name, const typename Data::name_type& data_name);

 private:
  PmidAccountHandler(const PmidAccountHandler&);
  PmidAccountHandler& operator=(const PmidAccountHandler&);
  PmidAccountHandler(PmidAccountHandler&&);
  PmidAccountHandler& operator=(PmidAccountHandler&&);

  const boost::filesystem::path kPmidAccountsRoot_;
  Db& db_;
  const NodeId kThisNodeId_;
  mutable std::mutex mutex_;
  AccountMap pmid_accounts_;
};

}  // namespace vault

}  // namespace maidsafe

#include "maidsafe/vault/pmid_account_holder/pmid_account_handler-inl.h"

#endif  // MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HANDLER_H_
