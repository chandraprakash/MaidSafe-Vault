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

#include "maidsafe/vault/maid_account_holder/maid_account.h"

#include <string>

#include "boost/variant/apply_visitor.hpp"

#include "maidsafe/common/utils.h"

#include "maidsafe/vault/demultiplexer.h"
#include "maidsafe/vault/pmid_account_holder/pmid_account.h"
#include "maidsafe/vault/maid_account_holder/maid_account_pb.h"


namespace fs = boost::filesystem;

namespace maidsafe {

namespace vault {

PmidTotals::PmidTotals() : serialised_pmid_registration(), pmid_record() {}

PmidTotals::PmidTotals(
    const nfs::PmidRegistration::serialised_type& serialised_pmid_registration_in,
    const PmidRecord& pmid_record_in)
        : serialised_pmid_registration(serialised_pmid_registration_in),
          pmid_record(pmid_record_in) {}

PmidTotals::PmidTotals(const PmidTotals& other)
    : serialised_pmid_registration(other.serialised_pmid_registration),
      pmid_record(other.pmid_record) {}

PmidTotals& PmidTotals::operator=(const PmidTotals& other) {
  serialised_pmid_registration = other.serialised_pmid_registration;
  pmid_record = other.pmid_record;
  return *this;
}

PmidTotals::PmidTotals(PmidTotals&& other)
    : serialised_pmid_registration(std::move(other.serialised_pmid_registration)),
      pmid_record(std::move(other.pmid_record)) {}

PmidTotals& PmidTotals::operator=(PmidTotals&& other) {
  serialised_pmid_registration = std::move(other.serialised_pmid_registration);
  pmid_record = std::move(other.pmid_record);
  return *this;
}



MaidAccount::PutDataDetails::PutDataDetails() : data_name_variant(), cost(0) {}


MaidAccount::PutDataDetails::PutDataDetails(const DataNameVariant& data_name_variant_in,
                                            int32_t cost_in)
    : data_name_variant(data_name_variant_in),
      cost(cost_in) {}

MaidAccount::PutDataDetails::PutDataDetails(const PutDataDetails& other)
  : data_name_variant(other.data_name_variant),
    cost(other.cost) {}

MaidAccount::PutDataDetails& MaidAccount::PutDataDetails::operator=(const PutDataDetails& other) {
  data_name_variant = other.data_name_variant;
  cost = other.cost;
  return *this;
}

MaidAccount::PutDataDetails::PutDataDetails(PutDataDetails&& other)
  : data_name_variant(std::move(other.data_name_variant)),
    cost(std::move(other.cost)) {}

MaidAccount::PutDataDetails& MaidAccount::PutDataDetails::operator=(PutDataDetails&& other) {
  data_name_variant = std::move(other.data_name_variant);
  cost = std::move(other.cost);
  return *this;
}



MaidAccount::MaidAccount(const MaidName& maid_name, const fs::path& root)
    : kMaidName_(maid_name),
      type_and_name_visitor_(),
      pmid_totals_(),
      recent_put_data_(),
      total_claimed_available_size_by_pmids_(0),
      total_put_data_(0),
      archive_(root / EncodeToBase32(kMaidName_.data)) {}

MaidAccount::MaidAccount(const serialised_type& serialised_maid_account, const fs::path& root)
    : kMaidName_([&serialised_maid_account]()->Identity {
                     protobuf::MaidAccount proto_maid_account;
                     proto_maid_account.ParseFromString(serialised_maid_account->string());
                     return Identity(proto_maid_account.maid_name());
                 }()),
      type_and_name_visitor_(),
      pmid_totals_(),
      recent_put_data_(),
      total_claimed_available_size_by_pmids_(0),
      total_put_data_(0),
      archive_(root / EncodeToBase32(kMaidName_.data)) {
  protobuf::MaidAccount proto_maid_account;
  if (!proto_maid_account.ParseFromString(serialised_maid_account->string())) {
    LOG(kError) << "Failed to parse maid_account.";
    ThrowError(CommonErrors::parsing_error);
  }

  for (int i(0); i != proto_maid_account.pmid_totals_size(); ++i) {
    pmid_totals_.emplace_back(
        nfs::PmidRegistration::serialised_type(NonEmptyString(
            proto_maid_account.pmid_totals(i).serialised_pmid_registration())),
        PmidRecord(proto_maid_account.pmid_totals(i).pmid_record()));
  }

  for (int i(0); i != proto_maid_account.recent_put_data_size(); ++i) {
    auto& recent_put_data(proto_maid_account.recent_put_data(i));
    recent_put_data_.emplace_back(
        GetDataNameVariant(static_cast<DataTagValue>(recent_put_data.type()),
                           Identity(recent_put_data.name())),
        recent_put_data.cost());
  }

  total_claimed_available_size_by_pmids_ =
      proto_maid_account.total_claimed_available_size_by_pmids();
  total_put_data_ = proto_maid_account.total_put_data();
}

MaidAccount::serialised_type MaidAccount::Serialise() const {
  protobuf::MaidAccount proto_maid_account;
  proto_maid_account.set_maid_name(kMaidName_->string());

  for (auto& pmid_total : pmid_totals_) {
    auto proto_pmid_totals(proto_maid_account.add_pmid_totals());
    proto_pmid_totals->set_serialised_pmid_registration(
        pmid_total.serialised_pmid_registration->string());
    *(proto_pmid_totals->mutable_pmid_record()) = pmid_total.pmid_record.ToProtobuf();
  }

  for (auto& recent_put_data_item : recent_put_data_) {
    auto proto_recent_put_data(proto_maid_account.add_recent_put_data());
    auto type_and_name(boost::apply_visitor(type_and_name_visitor_,
                                            recent_put_data_item.data_name_variant));
    proto_recent_put_data->set_type(static_cast<int32_t>(type_and_name.first));
    proto_recent_put_data->set_name(type_and_name.second.string());
    proto_recent_put_data->set_cost(recent_put_data_item.cost);
  }

  proto_maid_account.set_total_claimed_available_size_by_pmids(
      total_claimed_available_size_by_pmids_);
  proto_maid_account.set_total_put_data(total_put_data_);

  auto archive_file_ids(GetArchiveFileIdenitities());
  for (auto& archive_file_id : archive_file_ids) {
    auto proto_archive_file_id(proto_maid_account.add_file_ids());
    proto_archive_file_id->set_index(archive_file_id.first);
    proto_archive_file_id->set_hash(archive_file_id.second.string());
  }

  return serialised_type(NonEmptyString(proto_maid_account.SerializeAsString()));
}

MaidAccount::serialised_info_type  MaidAccount::SerialiseAccountSyncInfo() const {
  protobuf::MaidAccount proto_maid_account;
  proto_maid_account.set_maid_name(kMaidName_->string());
  for (auto& pmid_total : pmid_totals_) {
    auto proto_pmid_totals(proto_maid_account.add_pmid_totals());
    proto_pmid_totals->set_serialised_pmid_registration(
        pmid_total.serialised_pmid_registration->string());
    *(proto_pmid_totals->mutable_pmid_record()) = pmid_total.pmid_record.ToProtobuf();
  }

  for (auto& recent_put_data_item : recent_put_data_) {
    auto proto_recent_put_data(proto_maid_account.add_recent_put_data());
    auto type_and_name(boost::apply_visitor(type_and_name_visitor_,
                                            recent_put_data_item.data_name_variant));
    proto_recent_put_data->set_type(static_cast<int32_t>(type_and_name.first));
    proto_recent_put_data->set_name(type_and_name.second.string());
    proto_recent_put_data->set_cost(recent_put_data_item.cost);
  }

  proto_maid_account.set_total_claimed_available_size_by_pmids(
      total_claimed_available_size_by_pmids_);
  proto_maid_account.set_total_put_data(total_put_data_);

  auto archive_file_ids(GetArchiveFileIdenitities());
  for (auto& archive_file_id : archive_file_ids) {
    auto proto_archive_file_id(proto_maid_account.add_file_ids());
    proto_archive_file_id->set_index(archive_file_id.first);
    proto_archive_file_id->set_hash(archive_file_id.second.string());
  }

  return serialised_info_type(NonEmptyString(proto_maid_account.SerializeAsString()));
}

std::pair<MaidAccount::AccountInfo, DiskBasedStorage::FileIdentities>
    MaidAccount::ParseAccountSyncInfo(const serialised_info_type& serialised_info) {
  protobuf::MaidAccount proto_maid_account;
  MaidAccount::AccountInfo account_info;
  proto_maid_account.ParseFromString(serialised_info->string());
  for (auto index(0); index < proto_maid_account.pmid_totals_size(); ++index) {
    auto proto_pmid_totals(proto_maid_account.pmid_totals(index));
    nfs::PmidRegistration::serialised_type serialised_pmid_registration(
        NonEmptyString(proto_pmid_totals.serialised_pmid_registration()));
    PmidRecord pmid_record(proto_pmid_totals.pmid_record());
    account_info.pmid_totals.push_back(PmidTotals(serialised_pmid_registration, pmid_record));
  }
  for (auto index(0); index < proto_maid_account.recent_put_data_size(); ++index) {
    auto data_name(GetDataNameVariant(
        static_cast<DataTagValue>(proto_maid_account.recent_put_data(index).type()),
        Identity(proto_maid_account.recent_put_data(index).name())));
    PutDataDetails put_data_details(data_name,
                                    proto_maid_account.recent_put_data(index).cost());
    account_info.recent_put_data.push_back(put_data_details);
  }
  account_info.total_claimed_available_size_by_pmids =
      proto_maid_account.total_claimed_available_size_by_pmids();
  account_info.total_put_data = proto_maid_account.total_put_data();
  DiskBasedStorage::FileIdentities file_identities;
  for (auto index(0); index < proto_maid_account.file_ids_size(); ++index) {
    file_identities[proto_maid_account.file_ids(index).index()] =
        Identity(proto_maid_account.file_ids(index).hash());
  }
  return std::make_pair(account_info, file_identities);
}

std::vector<PmidTotals>::iterator MaidAccount::Find(const PmidName& pmid_name) {
  return std::find_if(pmid_totals_.begin(),
                      pmid_totals_.end(),
                      [&pmid_name](const PmidTotals& pmid_totals) {
                        return pmid_name == pmid_totals.pmid_record.pmid_name;
                      });
}

void MaidAccount::RegisterPmid(
    const nfs::PmidRegistration& pmid_registration) {
  auto itr(Find(pmid_registration.pmid_name()));
  if (itr == pmid_totals_.end()) {
    nfs::PmidRegistration::serialised_type serialised_pmid_registration(
        pmid_registration.Serialise());
    pmid_totals_.emplace_back(serialised_pmid_registration,
                              PmidRecord(pmid_registration.pmid_name()));
  }
}

void MaidAccount::UnregisterPmid(const PmidName& pmid_name) {
  auto itr(Find(pmid_name));
  if (itr != pmid_totals_.end())
    pmid_totals_.erase(itr);
}

void MaidAccount::UpdatePmidTotals(const PmidTotals& pmid_totals) {
  auto itr(Find(pmid_totals.pmid_record.pmid_name));
  if (itr == pmid_totals_.end())
    ThrowError(CommonErrors::no_such_element);
  *itr = pmid_totals;
}

DiskBasedStorage::FileIdentities MaidAccount::GetArchiveFileIdenitities() const {
  auto future(archive_.GetFileIdentities());
  return future.get();
}

NonEmptyString MaidAccount::GetArchiveFile(const DiskBasedStorage::FileIdentity& file_id) const {
  auto future(archive_.GetFile(file_id));
  return future.get();
}

void MaidAccount::PutArchiveFile(const DiskBasedStorage::FileIdentity& file_id,
                                 const NonEmptyString& content) {
  archive_.PutFile(file_id, content);
}

}  // namespace vault

}  // namespace maidsafe
