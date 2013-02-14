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

#ifndef MAIDSAFE_VAULT_PMID_ACCOUNT_H_
#define MAIDSAFE_VAULT_PMID_ACCOUNT_H_

#include <cstdint>
#include <deque>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/types.h"

#include "maidsafe/vault/disk_based_storage.h"
#include "maidsafe/vault/pmid_record.h"
#include "maidsafe/vault/types.h"


namespace maidsafe {

namespace vault {

class PmidAccount {
 public:
  struct DataElement {
    DataElement();
    DataElement(const DataNameVariant& data_name_variant_in, int32_t size_in);
    DataElement(const DataElement& other);
    DataElement& operator=(const DataElement& other);
    DataElement(DataElement&& other);
    DataElement& operator=(DataElement&& other);

    DataNameVariant data_name_variant;
    int32_t size;
  };
  enum class DataHolderStatus : int32_t { kDown, kGoingDown, kUp, kGoingUp };
  typedef PmidName name_type;
  typedef TaggedValue<NonEmptyString, struct SerialisedPmidAccountTag> serialised_type;
  PmidAccount(const PmidName& pmid_name, const boost::filesystem::path& root);
  PmidAccount(const serialised_type& serialised_pmid_account, const boost::filesystem::path& root);

  // Returns true if notification to MetadataManagers should begin (i.e. state changed from kDown)
  bool DataHolderUp();
  // Returns true if notification to MetadataManagers should begin (i.e. state changed from kUp)
  bool DataHolderDown();
  int32_t GetArchiveFileCount() const;
  std::vector<DataElement> ParseArchiveFile(int32_t index) const;

  // Used when this vault is no longer responsible for the account
  void ArchiveAccount();
  void RestoreAccount();

  // Used in synchronisation with other Pmid Account Holders - serialises all in-memory data
  serialised_type Serialise() const;
  std::vector<boost::filesystem::path> GetArchiveFileNames() const;
  NonEmptyString GetArchiveFile(const boost::filesystem::path& path) const;
  void PutArchiveFile(const boost::filesystem::path& path, const NonEmptyString& content);

  // Throw if the data is a duplicate
  template<typename Data>
  std::future<void> PutData(const typename Data::name_type& name, int32_t size);
  template<typename Data>
  std::future<void> DeleteData(const typename Data::name_type& name);

  name_type name() const { return pmid_record_.pmid_name; }
  DataHolderStatus data_holder_status() const { return data_holder_status_; }

 private:
  PmidAccount(const PmidAccount&);
  PmidAccount& operator=(const PmidAccount&);
  PmidAccount(PmidAccount&&);
  PmidAccount& operator=(PmidAccount&&);

  // Used when Data Holder has gone down, but this vault is still responsible for the account.
  void ArchiveRecentData();
  void RestoreRecentData();

  PmidRecord pmid_record_;
  DataHolderStatus data_holder_status_;
  std::deque<DataElement> recent_data_stored_;
  DiskBasedStorage archive_;
};

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_PMID_ACCOUNT_H_