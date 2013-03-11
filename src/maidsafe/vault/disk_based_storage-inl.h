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

#ifndef MAIDSAFE_VAULT_DISK_BASED_STORAGE_INL_H_
#define MAIDSAFE_VAULT_DISK_BASED_STORAGE_INL_H_

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

#include "maidsafe/common/utils.h"
#include "maidsafe/common/on_scope_exit.h"

#include "maidsafe/vault/parameters.h"


namespace maidsafe {

namespace vault {

namespace detail {

void SortElements(std::vector<protobuf::DiskStoredElement>& elements);

void SortFile(protobuf::DiskStoredFile& file);

}  // namespace detail

template<typename Data>
std::future<void> DiskBasedStorage::Store(const typename Data::name_type& name, int32_t value) {
  auto promise(std::make_shared<std::promise<void>>());
  active_.Send([name, value, promise, this] {
                 try {
                   if (!kAllowDuplicates_ && this->FindElement<Data>(name))
                     ThrowError(VaultErrors::unique_data_clash);
                   this->AddToLatestFile<Data>(name, value);
                   promise->set_value();
                 }
                 catch(const std::exception& e) {
                   LOG(kError) << "Execution of Store threw: " << e.what();
                   promise->set_exception(std::current_exception());
                 }
               });
  return promise->get_future();
}

template<typename Data>
bool DiskBasedStorage::FindElement(const typename Data::name_type& name,
                                   DiskBasedStorage::FileIdentity* file_id,
                                   int* index,
                                   protobuf::DiskStoredFile* file) {
  auto ritr(file_ids_.rbegin());
  if (!(*ritr).second.IsInitialised())
    ++ritr;

  for (; ritr != file_ids_.rend(); ++ritr) {
    auto disk_stored_file(ParseFile(*ritr));
    int entry_index(GetEntryIndex<Data>(name, disk_stored_file));
    if (entry_index >= 0) {
      if (file_id) {
        assert(index && file);
        *file_id = *ritr;
        *index = entry_index;
        *file = disk_stored_file;
      }
      return true;
    }
  }
  return false;
}

template<typename Data>
void DiskBasedStorage::AddToLatestFile(const typename Data::name_type& name, int32_t value) {
  std::pair<int, crypto::SHA512Hash> file_id(*file_ids_.rbegin());
  auto file(ParseFile(file_id));
  if (file.element_size() == detail::Parameters::max_file_element_count) {
    ++file_id.first;
    file_id.second = std::move(crypto::SHA512Hash());
    file_ids_.insert(file_id);
    file.Clear();
  }
  AddElement<Data>(name, value, file_id, file);
}

template<typename Data>
void DiskBasedStorage::AddElement(const typename Data::name_type& name,
                                  int32_t value,
                                  const FileIdentity& file_id,
                                  protobuf::DiskStoredFile& file) {
  auto proto_element(file.add_element());
  proto_element->set_type(static_cast<int32_t>(Data::type_enum_value()));
  proto_element->set_name(name->string());
  proto_element->set_value(value);
  assert(file.element_size() <= detail::Parameters::max_file_element_count);
  SaveChangedFile(file_id, file);
}

template<typename Data>
std::future<int32_t> DiskBasedStorage::Delete(const typename Data::name_type& name) {
  auto promise(std::make_shared<std::promise<int32_t>>());
  active_.Send([name, promise, this] {
                 try {
                   int32_t value(0);
                   this->FindAndDeleteEntry<Data>(name, value);
                   promise->set_value(value);
                 }
                 catch(const std::exception& e) {
                   LOG(kError) << "Execution of Delete threw: " << e.what();
                   promise->set_exception(std::current_exception());
                 }
               });
  return promise->get_future();
}

template<typename Data>
void DiskBasedStorage::FindAndDeleteEntry(const typename Data::name_type& name, int32_t& value) {
  FileIdentity file_id;
  int index(0);
  protobuf::DiskStoredFile file;
  if (!this->FindElement<Data>(name, &file_id, &index, &file))
    ThrowError(CommonErrors::no_such_element);

  value = file.element(index).value();
  DeleteEntry(index, file);
  SaveChangedFile(file_id, file);
}

template<typename Data>
int DiskBasedStorage::GetEntryIndex(const typename Data::name_type& name,
                                    const protobuf::DiskStoredFile& file) const {
  for (int i(0); i != file.element_size(); ++i) {
    if (file.element(i).type() == static_cast<int32_t>(Data::type_enum_value()) &&
        file.element(i).name() == name->string()) {
      return i;
    }
  }
  return -1;
}

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_DISK_BASED_STORAGE_INL_H_
