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

#include "maidsafe/vault/pmid_account_holder/pmid_record.h"

#include "maidsafe/common/log.h"

#include "maidsafe/vault/pmid_account_holder/pmid_account_pb.h"


namespace maidsafe {

namespace vault {

PmidRecord::PmidRecord()
    : pmid_name(),
      historic_stored_space(0),
      historic_lost_space(0),
      claimed_available_size(0) {}

PmidRecord::PmidRecord(const PmidName& pmid_name_in)
    : pmid_name(pmid_name_in),
      historic_stored_space(0),
      historic_lost_space(0),
      claimed_available_size(0) {}

PmidRecord::PmidRecord(const protobuf::PmidRecord& proto_pmid_record)
    : pmid_name(),
      historic_stored_space(0),
      historic_lost_space(0),
      claimed_available_size(0) {
  if (!proto_pmid_record.IsInitialized()) {
    LOG(kError) << "Failed to construct pmid_record.";
    ThrowError(CommonErrors::invalid_parameter);
  }
  pmid_name = PmidName(Identity(proto_pmid_record.pmid_name()));
  historic_stored_space = proto_pmid_record.historic_stored_space();
  historic_lost_space = proto_pmid_record.historic_lost_space();
  claimed_available_size = proto_pmid_record.claimed_available_size();
}

protobuf::PmidRecord PmidRecord::ToProtobuf() const {
  protobuf::PmidRecord proto_pmid_record;
  proto_pmid_record.set_pmid_name(pmid_name->string());
  proto_pmid_record.set_historic_stored_space(historic_stored_space);
  proto_pmid_record.set_historic_lost_space(historic_lost_space);
  proto_pmid_record.set_claimed_available_size(claimed_available_size);
  return proto_pmid_record;
}

PmidRecord::PmidRecord(const PmidRecord& other)
    : pmid_name(other.pmid_name),
      historic_stored_space(other.historic_stored_space),
      historic_lost_space(other.historic_lost_space),
      claimed_available_size(other.claimed_available_size) {}

PmidRecord& PmidRecord::operator=(const PmidRecord& other) {
  pmid_name = other.pmid_name;
  historic_stored_space = other.historic_stored_space;
  historic_lost_space = other.historic_lost_space;
  claimed_available_size = other.claimed_available_size;
  return *this;
}

PmidRecord::PmidRecord(PmidRecord&& other)
    : pmid_name(std::move(other.pmid_name)),
      historic_stored_space(std::move(other.historic_stored_space)),
      historic_lost_space(std::move(other.historic_lost_space)),
      claimed_available_size(std::move(other.claimed_available_size)) {}

PmidRecord& PmidRecord::operator=(PmidRecord&& other) {
  pmid_name = std::move(other.pmid_name);
  historic_stored_space = std::move(other.historic_stored_space);
  historic_lost_space = std::move(other.historic_lost_space);
  claimed_available_size = std::move(other.claimed_available_size);
  return *this;
}

}  // namespace vault

}  // namespace maidsafe
