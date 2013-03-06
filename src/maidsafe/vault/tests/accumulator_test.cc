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

#include "maidsafe/vault/accumulator.h"

#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/data_types/data_type_values.h"
#include "maidsafe/nfs/data_message.h"
#include "maidsafe/nfs/types.h"


namespace maidsafe {

namespace vault {

namespace test {

namespace {

nfs::DataMessage::Action GenerateAction() {
  return static_cast<nfs::DataMessage::Action>(RandomUint32() % 3);
}

nfs::PersonaId GenerateSource() {
  nfs::PersonaId source;
  // matches Persona enum in types.h
  source.persona = static_cast<nfs::Persona>(RandomUint32() % 7);
  source.node_id = NodeId(NodeId::kRandomId);
  return source;
}
nfs::DataMessage MakeMessage() {
  nfs::DataMessage::Data data(static_cast<DataTagValue>(RandomUint32() % 13),
                              Identity(RandomString(NodeId::kSize)),
                              NonEmptyString(RandomString(1 + RandomUint32() % 50)),
                              GenerateAction());
  nfs::DataMessage data_message(
      static_cast<nfs::Persona>(RandomUint32() % 7),
      GenerateSource(),
      data,
      passport::PublicPmid::name_type(Identity(RandomString(NodeId::kSize))));
  return data_message;
}
}  // unnamed namespace

TEST(AccumulatorTest, BEH_PushSingleResult) {
  nfs::DataMessage data_message = MakeMessage();
  maidsafe_error reply_code(CommonErrors::success);
  nfs::Reply reply(CommonErrors::success);
  Accumulator<passport::PublicMaid::name_type> accumulator;
  accumulator.PushSingleResult(data_message, [](const std::string&) {}, reply_code);
  EXPECT_EQ(accumulator.pending_requests_.size(), 1);
  // auto request_identity(accumulator.pending_requests_.at(0).first);
  EXPECT_FALSE(accumulator.CheckHandled(data_message, reply));
  accumulator.SetHandled(data_message, reply_code);
  EXPECT_EQ(accumulator.pending_requests_.size(), 0);
  EXPECT_TRUE(accumulator.CheckHandled(data_message, reply));
  Accumulator<passport::PublicMaid::name_type>::serialised_requests serialised(
      accumulator.Serialise(passport::PublicMaid::name_type(
      Identity((data_message.source().node_id).string()))));
  auto parsed(ParseHandledRequest<MaidName>(serialised));
  EXPECT_EQ(parsed.size(), 1);
}

TEST(AccumulatorTest, BEH_PushSingleResultThreaded) {
  maidsafe::test::RunInParallel(10, [] {
    nfs::DataMessage data_message = MakeMessage();
  maidsafe_error reply_code(CommonErrors::success);
  nfs::Reply reply(CommonErrors::success);
  Accumulator<passport::PublicMaid::name_type> accumulator;
  accumulator.PushSingleResult(data_message, [](const std::string&) {}, reply_code);
  EXPECT_EQ(accumulator.pending_requests_.size(), 1);
  // auto request_identity(accumulator.pending_requests_.at(0).first);
  EXPECT_FALSE(accumulator.CheckHandled(data_message, reply));
  accumulator.SetHandled(data_message, reply_code);
  EXPECT_EQ(accumulator.pending_requests_.size(), 0);
  EXPECT_TRUE(accumulator.CheckHandled(data_message, reply));
  Accumulator<passport::PublicMaid::name_type>::serialised_requests serialised(
      accumulator.Serialise(passport::PublicMaid::name_type(
      Identity((data_message.source().node_id).string()))));
  auto parsed(ParseHandledRequest<MaidName>(serialised));
  EXPECT_EQ(parsed.size(), 1);
    });
}

TEST(AccumulatorTest, BEH_CheckPendingRequestsLimit) {
  Accumulator<passport::PublicPmid::name_type> accumulator;
  //  Pending list limit 300
  size_t pending_request_max_limit = accumulator.kMaxPendingRequestsCount_;
  for (size_t index = 0; index < pending_request_max_limit; ++index) {
    nfs::DataMessage data_message = MakeMessage();
    nfs::Reply reply(CommonErrors::success);
    maidsafe_error reply_code(CommonErrors::success);
    accumulator.PushSingleResult(data_message, [](const std::string&) {},
                                reply_code);
    EXPECT_EQ(accumulator.pending_requests_.size(), (index + 1));
  }
  // Try to add request beyond the limit and it should fail
  EXPECT_EQ(accumulator.pending_requests_.size(), pending_request_max_limit);

  nfs::DataMessage data_message = MakeMessage();
  maidsafe_error reply_code(CommonErrors::success);
  accumulator.PushSingleResult(data_message, [](const std::string&) {},
                               reply_code);
  EXPECT_EQ(accumulator.pending_requests_.size(), pending_request_max_limit);
}

TEST(AccumulatorTest, BEH_CheckHandled) {
  nfs::DataMessage data_message = MakeMessage();
  maidsafe_error reply_code(CommonErrors::success);
  nfs::Reply reply(CommonErrors::success);
  Accumulator<passport::PublicMaid::name_type> accumulator;
  EXPECT_FALSE(accumulator.CheckHandled(data_message, reply));
  accumulator.PushSingleResult(data_message, [](const std::string&) {},
                               reply_code);
  accumulator.SetHandled(data_message, reply_code);
  EXPECT_TRUE(accumulator.CheckHandled(data_message, reply));
}

TEST(AccumulatorTest, BEH_SetHandled) {
  nfs::DataMessage data_message = MakeMessage();
  nfs::Reply reply(CommonErrors::success);
  maidsafe_error reply_code(CommonErrors::success);
  Accumulator<passport::PublicPmid::name_type> accumulator;
  EXPECT_TRUE(accumulator.handled_requests_.empty());
  accumulator.SetHandled(data_message, reply_code);
  EXPECT_EQ(accumulator.handled_requests_.size(), 1);
  accumulator.PushSingleResult(data_message, [](const std::string&) {},
                               reply_code);
  EXPECT_TRUE(accumulator.pending_requests_.empty());
  accumulator.SetHandled(data_message, reply_code);
  EXPECT_EQ(accumulator.handled_requests_.size(), 2);
  EXPECT_TRUE(accumulator.pending_requests_.empty());
}

TEST(AccumulatorTest, BEH_FindHandled) {
  nfs::DataMessage data_message = MakeMessage();
  maidsafe_error reply_code(CommonErrors::success);
  Accumulator<passport::PublicPmid::name_type> accumulator;
  EXPECT_TRUE(accumulator.handled_requests_.empty());
  auto itr_handle = accumulator.FindHandled(data_message);
  EXPECT_TRUE(itr_handle == accumulator.handled_requests_.end());
  accumulator.SetHandled(data_message, reply_code);
  EXPECT_EQ(accumulator.handled_requests_.size(), 1);
  itr_handle = accumulator.FindHandled(data_message);
  EXPECT_TRUE(itr_handle != accumulator.handled_requests_.end());
}


}  // namespace test

}  // namespace vault

}  // namespace maidsafe
