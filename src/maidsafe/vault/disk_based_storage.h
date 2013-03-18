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

#ifndef MAIDSAFE_VAULT_DISK_BASED_STORAGE_H_
#define MAIDSAFE_VAULT_DISK_BASED_STORAGE_H_

#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "maidsafe/common/active.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/types.h"

#include "maidsafe/vault/disk_based_storage.pb.h"


namespace maidsafe {

namespace vault {

namespace test {

class StandAloneDiskStorageTest_BEH_FileHandlers_Test;
template<typename T>
class DiskStorageTest;

}  // namespace test

class DiskBasedStorage {
 public:
  typedef std::map<int, crypto::SHA512Hash> FileIdentities;
  typedef FileIdentities::value_type FileIdentity;

  explicit DiskBasedStorage(const boost::filesystem::path& root);

  // Element handling
  template<typename Data>
  std::future<void> Store(const typename Data::name_type& name, int32_t value);
  // This returns the value which is being deleted, or throws if not found.
  template<typename Data>
  std::future<int32_t> Delete(const typename Data::name_type& name);

  // Synchronisation helpers
  std::future<uint32_t> GetFileCount() const;
  std::future<FileIdentities> GetFileIdentities() const;
  std::future<NonEmptyString> GetFile(const FileIdentity& file_id) const;
  void PutFile(const FileIdentity& file_id, const NonEmptyString& content);

  template<typename T>
  friend class test::DiskStorageTest;
  friend class test::StandAloneDiskStorageTest_BEH_FileHandlers_Test;

 private:
  DiskBasedStorage(const DiskBasedStorage&);
  DiskBasedStorage& operator=(const DiskBasedStorage&);
  DiskBasedStorage(DiskBasedStorage&&);
  DiskBasedStorage& operator=(DiskBasedStorage&&);

  FileIdentity GetAndVerifyFileNameParts(boost::filesystem::directory_iterator itr) const;

  boost::filesystem::path GetFileName(const FileIdentity& file_id) const;
  protobuf::DiskStoredFile ParseFile(const FileIdentity& file_id) const;

  template<typename Data>
  void AddToLatestFile(const typename Data::name_type& name, int32_t value);
  template<typename Data>
  void AddElement(const typename Data::name_type& name,
                  int32_t value,
                  const FileIdentity& file_id,
                  protobuf::DiskStoredFile& file);

  template<typename Data>
  void FindAndDeleteEntry(const typename Data::name_type& name, int32_t& value);
  template<typename Data>
  int GetEntryIndex(const typename Data::name_type& name,
                    const protobuf::DiskStoredFile& file) const;
  void DeleteEntry(int index, protobuf::DiskStoredFile& file) const;

  void SaveChangedFile(const FileIdentity& file_id, const protobuf::DiskStoredFile& file);

  FileIdentities::iterator GetReorganiseStartPoint();
  void ReadIntoMemory(FileIdentities::iterator &read_itr,
                      std::vector<protobuf::DiskStoredElement>& elements);
  void WriteToDisk(FileIdentities::iterator &write_itr,
                   std::vector<protobuf::DiskStoredElement>& elements);
  void PruneFilesToEnd(const FileIdentities::iterator& first_itr);

  void DoPutFile(const boost::filesystem::path& filename,
                 const NonEmptyString& content,
                 const FileIdentity& file_id);

  const boost::filesystem::path kRoot_;
  mutable Active active_;
  FileIdentities file_ids_;
};

}  // namespace vault

}  // namespace maidsafe

#include "maidsafe/vault/disk_based_storage-inl.h"

#endif  // MAIDSAFE_VAULT_DISK_BASED_STORAGE_H_
