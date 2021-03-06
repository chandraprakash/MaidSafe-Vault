/* Copyright 2012 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_VAULT_METADATA_MANAGER_METADATA_HANDLER_INL_H_
#define MAIDSAFE_VAULT_METADATA_MANAGER_METADATA_HANDLER_INL_H_

#include <set>
#include <string>
#include <vector>

#include "boost/filesystem/operations.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/vault/unresolved_element.pb.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace vault {

namespace detail {

boost::filesystem::path GetPath(const std::string& data_name,
                                int32_t data_type_enum_value,
                                const boost::filesystem::path& root);

template<typename Data>
boost::filesystem::path GetPath(const typename Data::name_type& data_name,
                                const boost::filesystem::path& root) {
  return GetPath(data_name->string(), static_cast<int>(Data::type_enum_value()), root);
}

//std::set<std::string> OnlinesToSet(const protobuf::Metadata& content);
//std::set<std::string> OfflinesToSet(const protobuf::Metadata& content);
//void OnlinesToProtobuf(const std::set<std::string>& onlines, protobuf::Metadata& content);
//void OfflinesToProtobuf(const std::set<std::string>& offlines, protobuf::Metadata& content);

}  // namespace detail

template<typename Data>
void MetadataHandler::IncrementSubscribers(const typename Data::name_type& /*data_name*/,
                                           int32_t /*data_size*/) {
//  Metadata<Data> metadata(data_name, metadata_db_.get(), data_size);
//  MetadataValueDelta metadata_value_delta;
//  metadata_value_delta.data_size = metadata.value_.data_size;
}

//template<typename Data>
//void MetadataHandler::IncrementSubscribers(const typename Data::name_type& data_name,
//                                           int32_t data_size) {
//  Metadata<Data> metadata(data_name, metadata_db_.get(), data_size);
//  ++metadata.value_.subscribers;
//  metadata.SaveChanges(metadata_db_.get());
//}

template<typename Data>
void MetadataHandler::DecrementSubscribers(const typename Data::name_type& /*data_name*/) {
//  Metadata<Data> metadata(data_name, metadata_db_.get());
//  --metadata.value_.subscribers;
//  metadata.SaveChanges(metadata_db_.get());
}

template<typename Data>
void MetadataHandler::DeleteMetadata(const typename Data::name_type& /*data_name*/) {
//  Metadata<Data> metadata(data_name, kMetadataRoot_);
//  metadata.value_.subscribers = 0;
//  metadata.SaveChanges();
}

template<typename Data>
void MetadataHandler::MarkNodeDown(const typename Data::name_type& /*data_name*/,
                                   const PmidName& /*pmid_name*/,
                                   int& /*remaining_online_holders*/) {
//  Metadata<Data> metadata(data_name, kMetadataRoot_);
//  metadata.value_.online_pmid_name.erase(pmid_name);
//  metadata.value_.offline_pmid_name.insert(pmid_name);
//  remaining_online_holders = metadata.value_.online_pmid_name.size();
//  metadata.SaveChanges();
}

template<typename Data>
void MetadataHandler::MarkNodeUp(const typename Data::name_type& /*data_name*/,
                                 const PmidName& /*pmid_name*/) {
//  Metadata<Data> metadata(data_name, kMetadataRoot_);
//  metadata.value_.online_pmid_name.insert(pmid_name);
//  metadata.value_.offline_pmid_name.erase(pmid_name);
//  metadata.SaveChanges();
}

template<typename Data>
void MetadataHandler::AddDataHolder(const typename Data::name_type& /*data_name*/,
                                    const PmidName& /*online_pmid_name*/) {
//  Metadata<Data> metadata(data_name, kMetadataRoot_);
//  metadata.value_.online_pmid_name.insert(online_pmid_name);
//  metadata.SaveChanges();
}

template<typename Data>
void MetadataHandler::RemoveDataHolder(const typename Data::name_type& /*data_name*/,
                                       const PmidName& /*pmid_name*/) {
//  Metadata<Data> metadata(data_name, kMetadataRoot_);
//  metadata.value_.online_pmid_name.erase(pmid_name);
//  metadata.value_.offline_pmid_name.erase(pmid_name);
//  metadata.SaveChanges();
}

template<typename Data>
std::vector<PmidName> MetadataHandler::GetOnlineDataHolders(
    const typename Data::name_type& data_name) const {
  DataNameVariant db_key(data_name);
  Metadata metadata(db_key, metadata_db_.get());
  std::vector<PmidName> onlines(metadata.value_.online_pmid_name.begin(),
                                metadata.value_.online_pmid_name.end());
  metadata.strong_guarantee_.Release();
  return onlines;
}


// Do we need to check unresolved list as well ?
template<typename Data>
bool MetadataHandler::CheckMetadataExists(const typename Data::name_type& data_name) const {
  try {
    DataNameVariant db_key(data_name);
    Metadata metadata(db_key, metadata_db_.get());
    metadata.strong_guarantee_.Release();
  } catch (const maidsafe_error& error) {
    return false;
  }
  return true;
}

template<typename Data>
std::pair<bool, int32_t> MetadataHandler::CheckPut(const typename Data::name_type& /*data_name*/, int32_t /*data_size*/) {
  return std::make_pair(false, 0);
}

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_METADATA_MANAGER_METADATA_HANDLER_INL_H_
