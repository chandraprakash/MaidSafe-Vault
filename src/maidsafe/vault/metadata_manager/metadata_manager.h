/* Copyright 2013 MaidSafe.net limited

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

#ifndef MAIDSAFE_VAULT_METADATA_MANAGER_METADATA_MANAGER_H_
#define MAIDSAFE_VAULT_METADATA_MANAGER_METADATA_MANAGER_H_

#include <cstdint>
#include <utility>

#include "maidsafe/common/types.h"
#include "maidsafe/nfs/types.h"
#include "maidsafe/vault/db_key.h"

namespace maidsafe {

namespace vault { struct MetadataValue; }

namespace nfs {

template<>
struct PersonaTypes<Persona::kMetadataManager> {
  typedef vault::DbKey RecordName;
  typedef vault::DbKey DbKey;
  typedef vault::MetadataValue DbValue;
  typedef std::pair<vault::DbKey, MessageAction> UnresolvedEntryKey;
  typedef DbValue UnresolvedEntryValue;
  static const Persona persona = Persona::kMetadataManager;
};

}  // namespace nfs

namespace vault {

typedef nfs::PersonaTypes<nfs::Persona::kMetadataManager> MetadataManager;

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_METADATA_MANAGER_METADATA_MANAGER_H_
