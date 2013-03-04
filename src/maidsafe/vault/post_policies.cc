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

#include "maidsafe/vault/post_policies.h"
#include "maidsafe/vault/types.h"


namespace maidsafe {

namespace vault {

void MaidAccountHolderManagementPolicy::GetPmidHealth(const PmidName& pmid_name,
                                                      const routing::ResponseFunctor& callback) {
  nfs::GenericMessage generic_message(nfs::GenericMessage::Action::kGetPmidHealth,
                                      nfs::Persona::kPmidAccountHolder,
                                      kSource_,
                                      pmid_name.data,
                                      NonEmptyString());
  nfs::Message message(nfs::GenericMessage::message_type_identifier,
                       generic_message.Serialise().data);
  routing_.SendGroup(NodeId(generic_message.name().string()), message.Serialise()->string(),
                     false, callback);
}

}  // namespace vault

}  // namespace maidsafe
