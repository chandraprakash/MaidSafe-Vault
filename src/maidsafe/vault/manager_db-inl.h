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

#ifndef MAIDSAFE_VAULT_MANAGER_DB_INL_H_
#define MAIDSAFE_VAULT_MANAGER_DB_INL_H_

#include "maidsafe/vault/manager_db.h"

#include <string>
#include <vector>

#include "boost/filesystem/operations.hpp"

#include "maidsafe/vault/utils.h"


namespace maidsafe {

namespace vault {

template<typename PersonaType>
ManagerDb<PersonaType>::ManagerDb(const boost::filesystem::path& path)
    : kDbPath_(path),
      mutex_(),
      leveldb_() {
  if (boost::filesystem::exists(kDbPath_))
    boost::filesystem::remove_all(kDbPath_);
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  options.error_if_exists = true;
  leveldb::Status status(leveldb::DB::Open(options, kDbPath_.string(), &db));
  if (!status.ok())
    ThrowError(CommonErrors::filesystem_io_error);
  leveldb_ = std::move(std::unique_ptr<leveldb::DB>(db));
  assert(leveldb_);
}

template<typename PersonaType>
ManagerDb<PersonaType>::~ManagerDb() {
  leveldb::DestroyDB(kDbPath_.string(), leveldb::Options());
}

template<typename PersonaType>
void ManagerDb<PersonaType>::Put(const KvPair& key_value_pair) {
  leveldb::Status status(leveldb_->Put(leveldb::WriteOptions(),
                                       key_value_pair.first.Serialise(),
                                       key_value_pair.second.Serialise()->string()));
  if (!status.ok())
    ThrowError(VaultErrors::failed_to_handle_request);
}

template<typename PersonaType>
void ManagerDb<PersonaType>::Delete(const typename PersonaType::DbKey& key) {
  leveldb::Status status(leveldb_->Delete(leveldb::WriteOptions(), key.Serialise()));
  if (!status.ok())
    ThrowError(VaultErrors::failed_to_handle_request);
}

template<typename PersonaType>
typename PersonaType::DbValue ManagerDb<PersonaType>::Get(const typename PersonaType::DbKey& key) {
  leveldb::ReadOptions read_options;
  read_options.verify_checksums = true;
  std::string value;
  leveldb::Status status(leveldb_->Get(read_options, key.Serialise(), &value));
  if (!status.ok())
    ThrowError(VaultErrors::failed_to_handle_request);
  assert(!value.empty());
  return typename PersonaType::DbValue(typename PersonaType::DbValue::serialised_type(
                                         NonEmptyString(value)));
}

// TODO(Team) This can be optimise by returning iterators.
template<typename PersonaType>
std::vector<typename PersonaType::DbKey> ManagerDb<PersonaType>::GetKeys() {
  std::vector<typename PersonaType::DbKey> return_vector;
  std::lock_guard<std::mutex> lock(mutex_);
  std::unique_ptr<leveldb::Iterator> iter(leveldb_->NewIterator(leveldb::ReadOptions()));
  for (iter->SeekToFirst(); iter->Valid(); iter->Next())
    return_vector.push_back(DbKey(iter->key().ToString()));
  return return_vector;
}

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_MANAGER_DB_INL_H_
