/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 * 
 * The MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "Operations.hpp"
#include "Wallet.hpp"

#include <graphene/utilities/key_conversion.hpp>

#include <fc/smart_ref_impl.hpp>

TransferOperation* OperationBuilder::transfer(ObjectId sender, ObjectId receiver, qint64 amount,
                                             ObjectId amountType, QString memo, ObjectId feeType)
{
   try {
      TransferOperation* op = new TransferOperation;
      op->setSender(sender);
      op->setReceiver(receiver);
      op->setAmount(amount);
      op->setAmountType(amountType);
      op->setMemo(memo);
      op->setFeeType(feeType);
      auto feeParameters = model.global_properties().parameters.current_fees->get<graphene::chain::transfer_operation>();
      op->setFee(op->operation().calculate_fee(feeParameters).value);
      return op;
   } catch (const fc::exception& e) {
      qDebug() << e.to_detail_string().c_str();
      return nullptr;
   }
}

QString TransferOperation::memo() const {
   if (!m_op.memo)
      return QString::null;
   if (memoIsEncrypted())
      return tr("Encrypted Memo");
   QString memo = QString::fromStdString(m_op.memo->get_message({}, {}));
   while (memo.endsWith(QChar('\0')))
      memo.chop(1);
   return memo;
}

bool TransferOperation::memoIsEncrypted() const
{
   if (!m_op.memo)
      return false;
   if (m_op.memo->message.empty())
      return false;
   if (m_op.memo->from == public_key_type() && m_op.memo->to == public_key_type())
      return false;
   return true;
}

bool TransferOperation::canEncryptMemo(Wallet* wallet, ChainDataModel* model) const
{
   if (!m_op.memo) return false;
   auto pub = model->getAccount(sender())->memoKey();
   if (!wallet->hasPrivateKey(pub)) return false;
   return graphene::utilities::wif_to_key(wallet->getPrivateKey(pub).toStdString()).valid();
}

bool TransferOperation::canDecryptMemo(Wallet* wallet, ChainDataModel* model) const
{
   if (!m_op.memo) return false;
   auto pub = model->getAccount(receiver())->memoKey();
   if (!wallet->hasPrivateKey(pub)) return false;
   return graphene::utilities::wif_to_key(wallet->getPrivateKey(pub).toStdString()).valid();
}

QString TransferOperation::decryptedMemo(Wallet* wallet, ChainDataModel* model) const
{
   fc::ecc::private_key privateKey;
   fc::ecc::public_key publicKey;

   if (canEncryptMemo(wallet, model)) {
      privateKey = *graphene::utilities::wif_to_key(wallet->getPrivateKey(model->getAccount(sender())->memoKey()).toStdString());
      publicKey = m_op.memo->to;
   } else if (canDecryptMemo(wallet, model)) {
      privateKey = *graphene::utilities::wif_to_key(wallet->getPrivateKey(model->getAccount(receiver())->memoKey()).toStdString());
      publicKey = m_op.memo->from;
   } else return QString::null;

   return QString::fromStdString(m_op.memo->get_message(privateKey, publicKey));
}

void TransferOperation::setMemo(QString memo) {
   if (memo == this->memo())
      return;
   if (!m_op.memo)
      m_op.memo = graphene::chain::memo_data();
   while (memo.size() % 32)
      memo.append('\0');
   m_op.memo->set_message({}, {}, memo.toStdString());
   Q_EMIT memoChanged();
}

void TransferOperation::encryptMemo(Wallet* wallet, ChainDataModel* model)
{
   if (!canEncryptMemo(wallet, model)) return;
   auto privateKey = graphene::utilities::wif_to_key(wallet->getPrivateKey(model->getAccount(sender())->memoKey()).toStdString());
   if (!privateKey) return;
   m_op.memo->set_message(*privateKey, public_key_type(model->getAccount(receiver())->memoKey().toStdString()), memo().toStdString());
   Q_EMIT memoChanged();
}
