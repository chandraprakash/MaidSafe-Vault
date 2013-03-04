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

#ifndef MAIDSAFE_VAULT_POST_POLICIES_H_
#define MAIDSAFE_VAULT_POST_POLICIES_H_

#include <string>
#include <vector>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/types.h"
#include "maidsafe/passport/types.h"
#include "maidsafe/routing/routing_api.h"
#include "maidsafe/nfs/message.h"
#include "maidsafe/nfs/generic_message.h"
#include "maidsafe/nfs/types.h"

#include "maidsafe/vault/sync.h"


namespace maidsafe {

namespace vault {

template <typename SyncPolicy, typename VaultManagement>
class VaultPostPolicy : public SyncPolicy, public VaultManagement {
 public:
  explicit VaultPostPolicy(routing::Routing& routing, const passport::Pmid& pmid)
      : SyncPolicy(routing, pmid),
        VaultManagement(routing, pmid) {}
};


template <nfs::Persona source_persona>
class SyncPolicy {
 public:
  SyncPolicy(routing::Routing& routing, const passport::Pmid& pmid)
      : routing_(routing),
        kSource_(source_persona, routing_.kNodeId()),
        kPmid_(pmid) {}

  template <typename Name>
  void PostSyncDataGroup(const Name& name,
                         const NonEmptyString& serialised_sync_data,
                         const routing::ResponseFunctor& callback) {
    nfs::GenericMessage generic_message(
        nfs::GenericMessage::Action::kSynchronise,
        source_persona,
        kSource_,
        name.data,
        serialised_sync_data);

    nfs::Message message(nfs::GenericMessage::message_type_identifier,
                         generic_message.Serialise().data);
    routing_.SendGroup(NodeId(generic_message.name().string()), message.Serialise()->string(),
                       false, callback);
  }

  void PostSyncDataDirect(const NodeId& target_node_id,
                          const NonEmptyString& serialised_sync_data,
                          const routing::ResponseFunctor& callback) {
    nfs::GenericMessage generic_message(
        nfs::GenericMessage::Action::kSynchronise,
        source_persona,
        kSource_,
        Identity(target_node_id.string()),
        serialised_sync_data);
    nfs::Message message(nfs::GenericMessage::message_type_identifier,
                         generic_message.Serialise().data);
    routing_.SendDirect(NodeId(generic_message.name().string()), message.Serialise()->string(),
                        false, callback);
  }

 private:
  routing::Routing& routing_;
  const nfs::PersonaId kSource_;
  const passport::Pmid kPmid_;
};

class MaidAccountHolderManagementPolicy {
 public:
  MaidAccountHolderManagementPolicy(routing::Routing& routing, const passport::Pmid& pmid)
      : routing_(routing),
        kSource_(nfs::Persona::kMaidAccountHolder, routing_.kNodeId()),
        kPmid_(pmid) {}
  void GetPmidHealth(const passport::PublicPmid::name_type& pmid_name,
                     const routing::ResponseFunctor& callback);

 private:
  routing::Routing& routing_;
  const nfs::PersonaId kSource_;
  const passport::Pmid kPmid_;
};

template <nfs::Persona source_persona>
class VaultManagement {
 public:
  VaultManagement(routing::Routing& routing, const passport::Pmid& pmid)
      : routing_(routing),
        kSource_(source_persona, routing_.kNodeId()),
        kPmid_(pmid) {}
// Below two methods should be moved to utils and classified methods need to
// be implemented per persona using common utility function

 private:
  void PostManagementMessageGroup(const nfs::GenericMessage generic_message,
                                  const routing::ResponseFunctor& callback) {
    nfs::Message message(nfs::GenericMessage::message_type_identifier,
                         generic_message.Serialise().data);
    routing_.SendGroup(NodeId(generic_message.name().string()), message.Serialise()->string(),
                         false, callback);
  }

  void PostManagementMessageDirect(const nfs::GenericMessage generic_message,
                                   const routing::ResponseFunctor& callback) {
    nfs::Message message(nfs::GenericMessage::message_type_identifier,
                         generic_message.Serialise().data);
    routing_.SendDirect(NodeId(generic_message.name().string()), message.Serialise()->string(),
                        false, callback);
  }

  routing::Routing& routing_;
  const nfs::PersonaId kSource_;
  const passport::Pmid kPmid_;
};

typedef VaultPostPolicy<SyncPolicy<nfs::Persona::kMaidAccountHolder>,
                        MaidAccountHolderManagementPolicy> MaidAccountHolderPostPolicy;

typedef VaultPostPolicy<SyncPolicy<nfs::Persona::kMetadataManager>,
    VaultManagement<nfs::Persona::kMetadataManager>> MetadataManagerPostPolicy;

typedef VaultPostPolicy<SyncPolicy<nfs::Persona::kPmidAccountHolder>,
    VaultManagement<nfs::Persona::kPmidAccountHolder>> PmidAccountHolderPostPolicy;

typedef VaultPostPolicy<SyncPolicy<nfs::Persona::kDataHolder>,
    VaultManagement<nfs::Persona::kDataHolder>> DataHolderPostPolicy;

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_POST_POLICIES_H_
