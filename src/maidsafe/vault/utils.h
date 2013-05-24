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

#ifndef MAIDSAFE_VAULT_UTILS_H_
#define MAIDSAFE_VAULT_UTILS_H_

#include <memory>
#include <mutex>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/data_types/data_name_variant.h"
#include "maidsafe/routing/routing_api.h"
#include "maidsafe/nfs/message.h"

#include "maidsafe/vault/types.h"


namespace maidsafe {

namespace vault {

template<typename name>
struct HandledRequest {
  HandledRequest(const nfs::MessageId& msg_id_in,
                  const name& account_name_in,
                  const nfs::MessageAction& action_type_in,
                  const Identity& data_name,
                  const DataTagValue& data_type,
                  const int32_t& size_in,
                  const maidsafe_error& return_code_in);
  HandledRequest(const HandledRequest& other);
  HandledRequest& operator=(const HandledRequest& other);
  HandledRequest(HandledRequest&& other);
  HandledRequest& operator=(HandledRequest&& other);

  nfs::MessageId msg_id;
  name account_name;
  nfs::MessageAction action;
  Identity data_name;
  DataTagValue data_type;
  int32_t size;
  maidsafe_error return_code;
};

struct CheckHoldersResult {
  std::vector<NodeId> new_holders;
  std::vector<NodeId> old_holders;
  routing::GroupRangeStatus proximity_status;
};

CheckHoldersResult CheckHolders(const routing::MatrixChange& matrix_change,
                                const NodeId& this_id,
                                const NodeId& target);

template<typename Message>
inline bool FromMaidAccountHolder(const Message& message);

template<typename Message>
inline bool FromMetadataManager(const Message& message);

template<typename Message>
inline bool FromPmidAccountHolder(const Message& message);

template<typename Message>
inline bool FromDataHolder(const Message& message);

template<typename Message>
inline bool FromClientMaid(const Message& message);

template<typename Message>
inline bool FromClientMpid(const Message& message);

template<typename Message>
inline bool FromOwnerDirectoryManager(const Message& message);

template<typename Message>
inline bool FromGroupDirectoryManager(const Message& message);

template<typename Message>
inline bool FromWorldDirectoryManager(const Message& message);

template<typename Message>
inline bool FromDataGetter(const Message& message);

template<nfs::Persona persona>
inline bool PendingRequestsEqual(const nfs::Message& lhs, const nfs::Message& rhs);

template<typename name>
bool PendingRequestsEqual(const nfs::Message& lhs, const nfs::Message& rhs);

namespace detail {

void InitialiseDirectory(const boost::filesystem::path& directory);

bool ShouldRetry(routing::Routing& routing, const nfs::Message& message);

template<typename Data>
bool IsDataElement(const typename Data::name_type& name,
                   const DataNameVariant& data_name_variant);

void SendReply(const nfs::Message& original_message,
               const maidsafe_error& return_code,
               const routing::ReplyFunctor& reply_functor);

template<typename AccountSet, typename Account>
typename Account::serialised_type GetSerialisedAccount(
    std::mutex& mutex,
    const AccountSet& accounts,
    const typename Account::name_type& account_name);

template<typename AccountSet, typename Account>
typename Account::serialised_info_type GetSerialisedAccountSyncInfo(
    std::mutex& mutex,
    const AccountSet& accounts,
    const typename Account::name_type& account_name);

// Returns true if the required successful request count has been reached
template<typename Accumulator>
bool AddResult(const nfs::Message& message,
               const routing::ReplyFunctor& reply_functor,
               const maidsafe_error& return_code,
               Accumulator& accumulator,
               std::mutex& accumulator_mutex,
               int requests_required);

}  // namespace detail

}  // namespace vault

}  // namespace maidsafe

#include "maidsafe/vault/utils-inl.h"

#endif  // MAIDSAFE_VAULT_UTILS_H_
