/**
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @brief Initializer for the ledger
 * @file LedgerInitializer.h
 * @author: yujiechen
 * @date 2021-06-10
 */
#pragma once
#include "Common.h"
#include "interfaces/ledger/LedgerInterface.h"
#include <bcos-framework/interfaces/protocol/BlockFactory.h>
#include <bcos-framework/interfaces/storage/StorageInterface.h>
#include <bcos-framework/libtool/NodeConfig.h>
#include <bcos-ledger/libledger/Ledger.h>

namespace bcos::initializer
{
class LedgerInitializer
{
public:
    static bcos::ledger::LedgerInterface::Ptr build(bcos::protocol::BlockFactory::Ptr _blockFactory,
        bcos::storage::StorageInterface::Ptr _storage, bcos::tool::NodeConfig::Ptr _nodeConfig)
    {
        auto ledger = std::make_shared<bcos::ledger::Ledger>(_blockFactory, _storage);
        // build genesis block
        ledger->buildGenesisBlock(_nodeConfig->ledgerConfig(), _nodeConfig->groupId(),
            _nodeConfig->txGasLimit(), _nodeConfig->genesisData());
        return ledger;
    }
};
}  // namespace bcos::initializer