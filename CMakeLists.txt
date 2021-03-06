#==================================================================================================#
#                                                                                                  #
#  Copyright 2012 MaidSafe.net limited                                                             #
#                                                                                                  #
#  This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or    #
#  later, and The General Public License (GPL), version 3. By contributing code to this project    #
#  You agree to the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in    #
#  the root directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also    #
#  available at:                                                                                   #
#                                                                                                  #
#    http://www.novinet.com/license                                                                #
#                                                                                                  #
#  Unless required by applicable law or agreed to in writing, software distributed under the       #
#  License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,       #
#  either express or implied. See the License for the specific language governing permissions      #
#  and limitations under the License.                                                              #
#                                                                                                  #
#==================================================================================================#


set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

cmake_minimum_required(VERSION 2.8.10 FATAL_ERROR)
project(vault)

include(../../cmake_modules/standard_setup.cmake)

include_directories(${common_SOURCE_DIR}/include)
include_directories(${passport_SOURCE_DIR}/include)
include_directories(${private_SOURCE_DIR}/include)
include_directories(${rudp_SOURCE_DIR}/include)
include_directories(${routing_SOURCE_DIR}/include)
include_directories(${nfs_SOURCE_DIR}/include)


#==================================================================================================#
# Set up all files as GLOBs                                                                        #
#==================================================================================================#
set(VaultSourcesDir ${PROJECT_SOURCE_DIR}/src/maidsafe/vault)
glob_dir(Vault ${VaultSourcesDir} Vault)
list(REMOVE_ITEM VaultSources ${VaultSourcesDir}/vault_main.cc)
glob_dir(DataHolder ${VaultSourcesDir}/data_holder "Data Holder\\\\")
glob_dir(MaidAccountHolder ${VaultSourcesDir}/maid_account_holder "Maid Account Holder\\\\")
glob_dir(MetadataManager ${VaultSourcesDir}/metadata_manager "Metadata Manager\\\\")
glob_dir(MpidAccountHolder ${VaultSourcesDir}/mpid_account_holder "Mpid Account Holder\\\\")
glob_dir(PmidAccountHolder ${VaultSourcesDir}/pmid_account_holder "Pmid Account Holder\\\\")
glob_dir(StructuredDataManager ${VaultSourcesDir}/structured_data_manager "Structured Data Manager\\\\")

glob_dir(VaultTests ${VaultSourcesDir}/tests "Tests")
glob_dir(VaultTools ${VaultSourcesDir}/tools "Tools")


#==================================================================================================#
# Define MaidSafe libraries and executables                                                        #
#==================================================================================================#
ms_add_static_library(vault ${VaultAllFiles}
                            ${DataHolderAllFiles}
                            ${MaidAccountHolderAllFiles}
                            ${MetadataManagerAllFiles}
                            ${MpidAccountHolderAllFiles}
                            ${PmidAccountHolderAllFiles}
                            ${StructuredDataManagerAllFiles})
ms_add_executable(lifestuff_vault Production ${VaultSourcesDir}/vault_main.cc)

target_link_libraries(maidsafe_vault maidsafe_nfs maidsafe_routing maidsafe_common maidsafe_private leveldb)
target_link_libraries(lifestuff_vault maidsafe_vault maidsafe_lifestuff_manager)

if(MaidsafeTesting)
  ms_add_executable(TESTvault "Tests/Vault" ${VaultTestsAllFiles})
  target_link_libraries(TESTvault maidsafe_vault)
  ms_add_executable(vault_key_helper Tools ${VaultToolsAllFiles})
  target_link_libraries(vault_key_helper maidsafe_routing maidsafe_nfs maidsafe_private)
endif()

rename_outdated_built_exes()


#==================================================================================================#
# Set compiler and linker flags                                                                    #
#==================================================================================================#
include(standard_flags)

if(MSVC AND MaidsafeTesting)
  set_target_properties(maidsafe_vault TESTvault PROPERTIES COMPILE_FLAGS "/bigobj")
endif()


#==================================================================================================#
# Tests                                                                                            #
#==================================================================================================#
if(MaidsafeTesting)
  add_style_test()
  add_gtests(TESTvault)
#   label_as_critical_tests(IdentityPacketsTest.BEH_Full
#                           IdentityPacketsTest.BEH_ChangeDetails
#                           PassportTest.BEH_SigningPackets
#                           PassportTest.BEH_IdentityPackets
#                           PassportTest.BEH_ChangingIdentityPackets
#                           PassportTest.BEH_MoveMaidsafeInbox
#                           PassportTest.BEH_SerialiseParse
#                           )
  add_project_experimental()
  test_summary_output()
endif()
