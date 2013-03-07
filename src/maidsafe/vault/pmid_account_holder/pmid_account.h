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

#ifndef MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_H_
#define MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_H_

#include <cstdint>
#include <deque>
#include <vector>
#include <utility>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/types.h"
#include "maidsafe/data_types/data_name_variant.h"

#include "maidsafe/vault/disk_based_storage.h"
#include "maidsafe/vault/pmid_account_holder/pmid_account_pb.h"
#include "maidsafe/vault/pmid_account_holder/pmid_record.h"
#include "maidsafe/vault/types.h"


namespace maidsafe {

namespace vault {

class PmidAccount {
 public:
  typedef PmidName name_type;
  typedef TaggedValue<NonEmptyString, struct SerialisedPmidAccountTag> serialised_type;

  PmidAccount(const PmidName& pmid_name, const boost::filesystem::path& root);
  PmidAccount(const serialised_type& serialised_pmid_account, const boost::filesystem::path& root);
  ~PmidAccount();

  void SetDataHolderUp();
  void SetDataHolderDown();

  int32_t GetArchiveFileCount() const { return archive_.GetFileCount().get(); }
  // Used when this vault is no longer responsible for the account
  void ArchiveAccount();
  void RestoreAccount();

  // Used in synchronisation with other Pmid Account Holders and also to periodically backup all
  // in-memory data to disk
  serialised_type SerialiseToDisk() const;
  std::vector<boost::filesystem::path> GetFileNames() const;
  NonEmptyString GetFile(const boost::filesystem::path& path) const;
  void PutFile(const boost::filesystem::path& path, const NonEmptyString& content);

  // Throw if the data is a duplicate
  template<typename Data>
  void PutData(const typename Data::name_type& name, int32_t size);
  template<typename Data>
  void DeleteData(const typename Data::name_type& name);

  name_type name() const { return pmid_record_.pmid_name; }
  bool DataHolderOnline() const { return data_holder_online_; }
  PmidRecord pmid_record() const { return pmid_record_; }

 private:
  PmidAccount(const PmidAccount&);
  PmidAccount& operator=(const PmidAccount&);
  PmidAccount(PmidAccount&&);
  PmidAccount& operator=(PmidAccount&&);

  // Only restore the latest, no need to add into pmid_record infos (i.e. total_stored_size)
  void RestoreRecentData();
  std::future<void> ArchiveDataRecord(const PmidAccount::DataElement data_element);

  const PmidName kAccountName_;
  const boost::filesystem::path kRoot_;
  int64_t historic_stored_space_;
  int64_t historic_lost_space_;
  int64_t claimed_available_size_;
  bool data_holder_online_;
  DiskBasedStorage data_held_;
};

}  // namespace vault

}  // namespace maidsafe

#include "maidsafe/vault/pmid_account_holder/pmid_account-inl.h"

#endif  // MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_H_
