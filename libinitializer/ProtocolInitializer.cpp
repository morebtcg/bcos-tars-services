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
 * @brief initializer for the protocol module
 * @file ProtocolInitializer.cpp
 * @author: yujiechen
 * @date 2021-06-10
 */
#include "ProtocolInitializer.h"
#include "../protocols/BlockImpl.h"
#include <bcos-crypto/encrypt/AESCrypto.h>
#include <bcos-crypto/encrypt/SM4Crypto.h>
#include <bcos-crypto/hash/Keccak256.h>
#include <bcos-crypto/hash/SM3.h>
#include <bcos-crypto/signature/secp256k1/Secp256k1Crypto.h>
#include <bcos-crypto/signature/sm2/SM2Crypto.h>
#include <bcos-framework/libprotocol/TransactionSubmitResultFactoryImpl.h>

using namespace bcos;
using namespace bcostars::protocol;
using namespace bcos::initializer;
using namespace bcos::crypto;
using namespace bcos::tool;

void ProtocolInitializer::init(NodeConfig::Ptr _nodeConfig, bool _loadKeyPair)
{
    // TODO: hsm/ed25519
    if (_nodeConfig->smCryptoType())
    {
        createSMCryptoSuite();
    }
    createCryptoSuite();
    INITIALIZER_LOG(INFO) << LOG_DESC("init crypto suite success");

    // create the block factory
    // TODO: pb/tars option
    auto blockHeaderFactory = std::make_shared<BlockHeaderFactoryImpl>(m_cryptoSuite);
    auto transactionFactory = std::make_shared<TransactionFactoryImpl>(m_cryptoSuite);
    auto receiptFactory = std::make_shared<TransactionReceiptFactoryImpl>(m_cryptoSuite);
    m_blockFactory = std::make_shared<BlockFactoryImpl>(
        m_cryptoSuite, blockHeaderFactory, transactionFactory, receiptFactory);

    m_cryptoSuite->setKeyFactory(_nodeConfig->keyFactory());
    m_txResultFactory = std::make_shared<bcos::protocol::TransactionSubmitResultFactoryImpl>();

    INITIALIZER_LOG(INFO) << LOG_DESC("init blockFactory success");

    if (_loadKeyPair)
    {
        loadKeyPair(_nodeConfig);
    }
}

void ProtocolInitializer::createCryptoSuite()
{
    auto hashImpl = std::make_shared<Keccak256>();
    auto signatureImpl = std::make_shared<Secp256k1Crypto>();
    auto encryptImpl = std::make_shared<AESCrypto>();
    m_cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, signatureImpl, encryptImpl);
}

void ProtocolInitializer::createSMCryptoSuite()
{
    auto hashImpl = std::make_shared<SM3>();
    auto signatureImpl = std::make_shared<SM2Crypto>();
    auto encryptImpl = std::make_shared<SM4Crypto>();
    m_cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, signatureImpl, encryptImpl);
}

void ProtocolInitializer::loadKeyPair(NodeConfig::Ptr _nodeConfig)
{
    auto privateKeyData = loadPrivateKey(_nodeConfig->privateKeyPath());
    auto privateKey = _nodeConfig->keyFactory()->createKey(*privateKeyData);
    m_keyPair = m_cryptoSuite->signatureImpl()->createKeyPair(privateKey);

    INITIALIZER_LOG(INFO) << LOG_DESC("loadKeyPair success")
                          << LOG_KV("privateKeyPath", _nodeConfig->privateKeyPath());
}