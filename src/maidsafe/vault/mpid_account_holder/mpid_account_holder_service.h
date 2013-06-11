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

#ifndef MAIDSAFE_VAULT_MPID_ACCOUNT_HOLDER_MPID_ACCOUNT_HOLDER_SERVICE_H_
#define MAIDSAFE_VAULT_MPID_ACCOUNT_HOLDER_MPID_ACCOUNT_HOLDER_SERVICE_H_

#include <vector>

#include "maidsafe/common/rsa.h"
#include "maidsafe/routing/routing_api.h"
#include "maidsafe/nfs/message.h"

// #include "maidsafe/vault/disk_based_storage.h"


namespace maidsafe {

namespace vault {

class MpidAccountHolder {
 public:
  MpidAccountHolder(routing::Routing& routing, const boost::filesystem::path& vault_root_dir);
  ~MpidAccountHolder();
  template<typename Data>
  void HandleMessage(const nfs::Message& message, const routing::ReplyFunctor& reply_functor);
  void HandleMessage(const nfs::Message& /*message*/, const routing::ReplyFunctor& /*reply_functor*/) {}
  void HandleChurnEvent(routing::MatrixChange matrix_change);

 private:
//  void HandlePutMessage(const Message& message);
//  void HandleGetMessage(const Message& message);
//  void HandlePostMessage(const Message& message);
//  void HandleDeleteMessage(const Message& message);
//  boost::filesystem::path vault_root_dir_;
//  routing::Routing& routing_;
//  DiskBasedStorage disk_storage_;
};

template<typename Data>
void MpidAccountHolder::HandleMessage(const nfs::Message& /*message*/,
                                      const routing::ReplyFunctor& /*reply_functor*/) {}

}  // namespace vault

}  // namespace maidsafe


#endif  // MAIDSAFE_VAULT_MPID_ACCOUNT_HOLDER_MPID_ACCOUNT_HOLDER_SERVICE_H_
