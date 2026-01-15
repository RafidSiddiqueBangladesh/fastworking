const mongoose = require('mongoose');

const transactionSchema = new mongoose.Schema({
  type: { type: String, enum: ['buy', 'sell'], required: true },
  payment: { type: String, enum: ['cash', 'due'], required: true },
  entity_id: { type: String, required: function() { return this.type === 'sell'; } }, // for sells
  product_id: { type: String, required: function() { return this.type === 'buy'; } }, // for buys
  amount: { type: Number, required: true },
  date: { type: Date, default: Date.now }
});

module.exports = mongoose.model('Transaction', transactionSchema);