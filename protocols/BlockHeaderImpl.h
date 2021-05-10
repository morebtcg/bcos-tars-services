#pragma once

#include "Block.h"
#include "Common.h"
#include <bcos-framework/interfaces/protocol/Block.h>
#include <bcos-framework/interfaces/protocol/BlockFactory.h>
#include <bcos-framework/interfaces/protocol/BlockHeader.h>
#include <bcos-framework/interfaces/protocol/BlockHeaderFactory.h>
#include <gsl/span>

namespace bcostars {
namespace protocol {

class BlockHeader : public bcos::protocol::BlockHeader {
public:
  virtual ~BlockHeader() {}

  BlockHeader(bcostars::BlockHeader *blockHeader)
      : m_inner(std::shared_ptr<bcostars::BlockHeader>(
            blockHeader, [](bcostars::BlockHeader *) {})){};

  virtual void decode(bcos::bytesConstRef _data) override {
    tars::TarsInputStream input;
    input.setBuffer((const char *)_data.data(), _data.size());

    m_inner->readFrom(input);
  }

  virtual void encode(bcos::bytes &_encodeData) const override {
    tars::TarsOutputStream<bcostars::protocol::BufferWriterByteVector> output;

    m_inner->writeTo(output);
    output.getByteBuffer().swap(_encodeData);
  }

  virtual bcos::crypto::HashType const& hash() const override {
    bcos::bytes encoded;
    encode(encoded);

    m_hash = m_cryptoSuite->hash(encoded);
    return m_hash;
  }
  void populateFromParents(BlockHeadersPtr _parents,
                           bcos::protocol::BlockNumber _number) override {
    (void)_parents;
    (void)_number;
  }
  void clear() override {}
  // verify the signatureList
  void verifySignatureList() const override {}
  void populateEmptyBlock(int64_t _timestamp) override;

  virtual int32_t version() const override { return m_inner->version; }
  gsl::span<const bcos::protocol::ParentInfo> parentInfo() const override {}
  bcos::crypto::HashType const &txsRoot() const override {
    return *(reinterpret_cast<bcos::crypto::HashType *>(
        m_inner->txsRoot.data()));
  }
  bcos::crypto::HashType const &receiptRoot() const override {
    return *(reinterpret_cast<bcos::crypto::HashType *>(
        m_inner->receiptRoot.data()));
  }
  bcos::crypto::HashType const &stateRoot() const override {
    return *(reinterpret_cast<bcos::crypto::HashType *>(
        m_inner->stateRoot.data()));
  }
  bcos::protocol::BlockNumber number() const override {
    return m_inner->blockNumber;
  }
  bcos::u256 const &gasUsed() override {
    return *((bcos::u256 *)(m_inner->gasUsed.data()));
  }
  int64_t timestamp() override { return m_inner->timestamp; }
  int64_t sealer() override { return m_inner->sealer; }
  gsl::span<const bcos::bytes> sealerList() const override {
    return m_inner->sealerList;
  }
  bcos::bytesConstRef extraData() const override {
    return bcos::bytesConstRef(m_inner->extraData.data(),
                               m_inner->extraData.size());
  }
  gsl::span<const bcos::protocol::Signature> signatureList() const override {
    return gsl::span(reinterpret_cast<const bcos::protocol::Signature *>(
                         m_inner->signatureList.data()),
                     m_inner->signatureList.size());
  }

  virtual gsl::span<const uint64_t> consensusWeights() const override {
    return gsl::span(
        reinterpret_cast<uint64_t *>(m_inner->consensusWeights.data()),
        m_inner->consensusWeights.size());
  }

  void setVersion(int32_t _version) override {
    m_inner->version = _version;
  }
  void setParentInfo(
      gsl::span<const bcos::protocol::ParentInfo> const &_parentInfo) override {
    m_inner->parentInfo.clear();
    for (auto &it : _parentInfo) {
      ParentInfo parentInfo;
      parentInfo.blockNumber = it.blockNumber;
      *parentInfo.blockHash = *it.blockHash.data();
      m_inner->parentInfo.emplace_back(parentInfo);
    }
  }
  virtual void setTxsRoot(bcos::crypto::HashType const &_txsRoot) override {
    m_inner->txsRoot.assign(_txsRoot.begin(), _txsRoot.end());
  }

  virtual void
  setReceiptRoot(bcos::crypto::HashType const &_receiptRoot) override {
    m_inner->receiptRoot.assign(_receiptRoot.begin(), _receiptRoot.end());
  }
  virtual void setStateRoot(bcos::crypto::HashType const &_stateRoot) override {
    m_inner->stateRoot.assign(_stateRoot.begin(), _stateRoot.end());
  }
  virtual void setNumber(bcos::protocol::BlockNumber _blockNumber) override {
    m_inner->blockNumber = _blockNumber;
  }
  virtual void setGasUsed(bcos::u256 const &_gasUsed) override {}
  virtual void setTimestamp(int64_t _timestamp) override {
    m_inner->timestamp = _timestamp;
  }
  virtual void setSealer(int64_t _sealerId) override {
    m_inner->sealer = _sealerId;
  }
  virtual void
  setSealerList(gsl::span<const bcos::bytes> const &_sealerList) override {
    m_inner->sealerList.assign(_sealerList.begin(), _sealerList.end());
  }

  virtual void
  setConsensusWeights(gsl::span<const uint64_t> const &_weightList) override {
    m_inner->consensusWeights.assign(_weightList.begin(),
                                           _weightList.end());
  }

  virtual void setExtraData(bcos::bytes const &_extraData) override {
    m_inner->extraData = _extraData;
  }
  virtual void setExtraData(bcos::bytes &&_extraData) override {
    m_inner->extraData = std::move(_extraData);
  }
  virtual void setSignatureList(gsl::span<const bcos::protocol::Signature> const
                                    &_signatureList) override {
    for (auto &it : _signatureList) {
      Signature signature;
      signature.sealerIndex = it.index;
      signature.signature = it.signature;
      m_inner->signatureList.emplace_back(signature);
    }
  }

private:
  std::shared_ptr<bcostars::BlockHeader> m_inner;
  mutable bcos::crypto::HashType m_hash;
  bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;
};

} // namespace protocol
} // namespace bcostars