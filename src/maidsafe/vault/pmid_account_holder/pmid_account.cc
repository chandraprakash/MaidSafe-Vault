/***************************************************************************************************
 *  Copyright 2012 maidsafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use. The use of this code is governed by the licence file licence.txt found in the root of     *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit written *
 *  permission of the board of directors of MaidSafe.net.                                          *
 **************************************************************************************************/

#include <string>

#include "maidsafe/data_types/data_name_variant.h"

#include "maidsafe/vault/pmid_account_holder/pmid_account.h"
#include "maidsafe/vault/utils.h"


namespace maidsafe {

namespace vault {

namespace {

//PmidRecord ParsePmidRecord(const PmidAccount::serialised_type& serialised_pmid_account) {
//  protobuf::PmidAccount pmid_account;
//  if (!pmid_account.ParseFromString(serialised_pmid_account.data.string()))
//    ThrowError(CommonErrors::parsing_error);
//  return PmidRecord(pmid_account.pmid_record());
//}

}  // namespace

PmidAccount::PmidAccount(const PmidName& pmid_name, const boost::filesystem::path& root)
    : kAccountName_(pmid_name),
      kRoot_(root / EncodeToBase32(pmid_name->string())),
      historic_stored_space_(0),
      historic_lost_space_(0),
      claimed_available_size_(0),
      data_holder_online_(true),
      data_held_(kRoot_, false) {}

PmidAccount::PmidAccount(const serialised_type& serialised_pmid_account,
                         const boost::filesystem::path& root)
    : pmid_record_(ParsePmidRecord(serialised_pmid_account)),
      data_holder_online_(true),
      kRoot_(root / EncodeToBase32(pmid_record_.pmid_name->string())),
      data_held_(kRoot_, false) {
  protobuf::PmidAccount pmid_account;
  if (!pmid_account.ParseFromString(serialised_pmid_account->string())) {
    LOG(kError) << "Failed to parse pmid_account.";
    ThrowError(CommonErrors::parsing_error);
  }
  for (auto& recent_data : pmid_account.recent_data_stored()) {
    DataElement data_element(GetDataNameVariant(static_cast<DataTagValue>(recent_data.type()),
                                                Identity(recent_data.name())),
                             recent_data.size());
    recent_data_stored_.push_back(data_element);
  }
}

PmidAccount::~PmidAccount() {
  ArchiveAccount();
}

void PmidAccount::SetDataHolderUp() {
  RestoreRecentData();
  data_holder_online_ = true;
}

void PmidAccount::SetDataHolderDown() {
  ArchiveRecentData();
  data_holder_online_ = false;
}

std::vector<PmidAccount::DataElement> PmidAccount::ParseArchiveFile(int32_t /*index*/) const {
//  std::vector<boost::filesystem::path> files(data_held_.GetFileNames().get());
  std::vector<PmidAccount::DataElement> data_elements;
//  for (auto& file : files) {
//    if (static_cast<size_t>(index) == ExtractFileIndexFromFilename(file.filename().string())) {
//      protobuf::DiskStoredFile archived_pmid_data;
//      archived_pmid_data.ParseFromString(data_held_.GetFile(file).get().string());
//      for (auto& data_record : archived_pmid_data.data_stored()) {
//        DataElement data_element(GetDataNameVariant(static_cast<DataTagValue>(data_record.type()),
//                                                    Identity(data_record.name())),
//                                 data_record.size());
//        data_elements.push_back(data_element);
//      }
//    }
//  }
  return data_elements;
}

void PmidAccount::ArchiveAccount() {
  try {
    ArchiveRecentData();
    crypto::SHA512Hash hash(crypto::Hash<crypto::SHA512>(pmid_record_.pmid_name->string()));
    std::string file_name(EncodeToBase32(hash));
    maidsafe::WriteFile(kRoot_ / file_name, pmid_record_.ToProtobuf().SerializeAsString());
  }
  catch(const std::exception& e) {
    LOG(kError) << "error in archive memory data or account info: " << e.what();
    ThrowError(CommonErrors::filesystem_io_error);
  }
}

void PmidAccount::RestoreAccount() {
  try {
    crypto::SHA512Hash hash(crypto::Hash<crypto::SHA512>(pmid_record_.pmid_name->string()));
    std::string file_name(EncodeToBase32(hash));
    NonEmptyString content(maidsafe::ReadFile(kRoot_ / file_name));
    protobuf::PmidRecord pmid_record;
    pmid_record.ParseFromString(content.string());
    pmid_record_ = PmidRecord(pmid_record);
    RestoreRecentData();
  }
  catch(const std::exception& e) {
    LOG(kError) << "error in restore archived data and account info: " << e.what();
    ThrowError(CommonErrors::filesystem_io_error);
  }
}

PmidAccount::serialised_type PmidAccount::Serialise() const {
  protobuf::PmidAccount pmid_account;
  *(pmid_account.mutable_pmid_record()) = pmid_record_.ToProtobuf();
  for (auto& record : recent_data_stored_)
    pmid_account.add_recent_data_stored()->CopyFrom(record.ToProtobuf());
  return serialised_type(NonEmptyString(pmid_account.SerializeAsString()));
}

std::vector<boost::filesystem::path> PmidAccount::GetArchiveFileNames() const {
  return data_held_.GetFileNames().get();
}

NonEmptyString PmidAccount::GetArchiveFile(const boost::filesystem::path& path) const {
  return data_held_.GetFile(path).get();
}

void PmidAccount::PutArchiveFile(const boost::filesystem::path& path,
                                 const NonEmptyString& content) {
  data_held_.PutFile(path, content);
}

protobuf::PmidTotals PmidAccount::pmid_totals() const {
  protobuf::PmidTotals proto_totals;
  proto_totals.set_pmid_name(kAccountName_->string());
  proto_totals.set_historic_stored_space(historic_stored_space_);
  proto_totals.set_historic_lost_space(historic_lost_space_);
  proto_totals.set_claimed_available_size(claimed_available_size_);
  return proto_totals;
}

}  // namespace vault

}  // namespace maidsafe
