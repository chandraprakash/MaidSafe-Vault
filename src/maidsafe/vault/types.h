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

#ifndef MAIDSAFE_VAULT_TYPES_H_
#define MAIDSAFE_VAULT_TYPES_H_

#include <functional>
#include <memory>
#include <set>

#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/types.h"
#include "maidsafe/passport/types.h"
#include "maidsafe/nfs/data_policies.h"
#include "maidsafe/nfs/nfs.h"
#include "maidsafe/nfs/types.h"

#include "maidsafe/vault/post_policies.h"


namespace maidsafe {

namespace vault {

class MaidAccount;
class PmidAccount;

typedef nfs::NetworkFileSystem<
    nfs::MaidAccountHolderPutPolicy,
    nfs::MaidAccountHolderGetPolicy,
    nfs::MaidAccountHolderDeletePolicy,
    MaidAccountHolderPostPolicy> MaidAccountHolderNfs;

typedef nfs::NetworkFileSystem<
    nfs::MetadataManagerPutPolicy,
    nfs::MetadataManagerGetPolicy,
    nfs::MetadataManagerDeletePolicy,
    MetadataManagerPostPolicy> MetadataManagerNfs;

typedef nfs::NetworkFileSystem<
    nfs::StructuredDataManagerPutPolicy,
    nfs::StructuredDataManagerGetPolicy,
    nfs::StructuredDataManagerDeletePolicy,
    StructuredDataManagerPostPolicy> StructuredDataManagerNfs;

typedef nfs::NetworkFileSystem<
    nfs::PmidAccountHolderPutPolicy,
    nfs::PmidAccountHolderGetPolicy,
    nfs::PmidAccountHolderDeletePolicy,
    PmidAccountHolderPostPolicy> PmidAccountHolderNfs;

// TODO(dirvine) BEFORE_RELEASE this is a hack to create a type for the dataholder, the proper
// implmentation is required ,,,,,,,,,,,,,,,,,,,,,
typedef nfs::NetworkFileSystem<
    nfs::PmidAccountHolderPutPolicy,
    nfs::PmidAccountHolderGetPolicy,
    nfs::PmidAccountHolderDeletePolicy,
    PmidAccountHolderPostPolicy> DataHolderNfs;

typedef passport::PublicMaid::name_type MaidName;
typedef passport::PublicPmid::name_type PmidName;
typedef passport::PublicMpid::name_type MpidName;

typedef std::set<std::unique_ptr<MaidAccount>,
                 std::function<bool(const std::unique_ptr<MaidAccount>&,
                                    const std::unique_ptr<MaidAccount>&)>> MaidAccountSet;
typedef std::set<std::unique_ptr<PmidAccount>,
                 std::function<bool(const std::unique_ptr<PmidAccount>&,
                                    const std::unique_ptr<PmidAccount>&)>> PmidAccountSet;

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_TYPES_H_
