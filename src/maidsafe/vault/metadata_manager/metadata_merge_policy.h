/***************************************************************************************************
 *  Copyright 2013 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#ifndef MAIDSAFE_VAULT_METADATA_MANAGER_METADATA_MERGE_POLICY_H_
#define MAIDSAFE_VAULT_METADATA_MANAGER_METADATA_MERGE_POLICY_H_

#include <map>
#include <set>
#include <utility>
#include <vector>

#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/types.h"
#include "maidsafe/nfs/types.h"

#include "maidsafe/vault/unresolved_element.h"
#include "maidsafe/vault/manager_db.h"
#include "maidsafe/vault/metadata_manager/metadata_manager.h"


namespace maidsafe {

namespace vault {

typedef UnresolvedElement<MetadataManager> MetadataUnresolvedEntry;
typedef void MetadataResolvedEntry;

class MetadataMergePolicy {
 public:
  typedef MetadataUnresolvedEntry UnresolvedEntry;
  typedef MetadataResolvedEntry ResolvedEntry;
  typedef MetadataManager::DbKey DbKey;
  typedef ManagerDb<MetadataManager> Database;

  explicit MetadataMergePolicy(ManagerDb<MetadataManager>* metadata_db);
  MetadataMergePolicy(MetadataMergePolicy&& other);
  MetadataMergePolicy& operator=(MetadataMergePolicy&& other);

 protected:
  typedef std::vector<UnresolvedEntry> UnresolvedEntries;
  typedef std::vector<UnresolvedEntry>::iterator UnresolvedEntriesItr;

  void Merge(const UnresolvedEntry& unresolved_entry);

  UnresolvedEntries unresolved_data_;
  ManagerDb<MetadataManager>* metadata_db_;

 private:
  MetadataMergePolicy(const MetadataMergePolicy&);
  MetadataMergePolicy& operator=(const MetadataMergePolicy&);

  void MergePut(const DataNameVariant& data_name, int data_size);
  void MergeDelete(const DataNameVariant& data_name, int data_size);
  int GetDataSize(const UnresolvedEntry& unresolved_entry) const;
  UnresolvedEntries MergeRecordTransfer(const UnresolvedEntry& unresolved_entry);
};

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_METADATA_MANAGER_METADATA_MERGE_POLICY_H_
