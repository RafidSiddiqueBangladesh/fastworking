const express = require('express');
const mongoose = require('mongoose');
const Transaction = require('./models/Transaction');

require('dotenv').config();

const app = express();

app.use(express.json());

// Connect to MongoDB - assuming local, change to your cloud URI
mongoose.connect(process.env.MONGO_URI, { useNewUrlParser: true, useUnifiedTopology: true })
  .then(() => console.log('MongoDB connected'))
  .catch(err => console.log(err));

// POST /api/transaction
app.post('/api/transaction', async (req, res) => {
  const { data } = req.body;
  if (!data) return res.status(400).json({ error: 'No data provided' });

  let type, payment, id, amount;
  if (data.startsWith('*')) {
    type = 'buy';
  } else if (data.startsWith('#')) {
    type = 'sell';
  } else {
    return res.status(400).json({ error: 'Invalid data format' });
  }

  const paymentChar = data[1];
  payment = paymentChar === '1' ? 'cash' : 'due';

  id = data.substr(2, 2);
  amount = parseInt(data.substr(4));

  if (isNaN(amount)) return res.status(400).json({ error: 'Invalid amount' });

  const transaction = new Transaction(type === 'buy' ? { type, payment, product_id: id, amount } : { type, payment, entity_id: id, amount });
  await transaction.save();

  res.json({ message: 'Transaction saved' });
});

// GET /api/sells
app.get('/api/sells', async (req, res) => {
  const today = new Date();
  today.setHours(0,0,0,0);
  const tomorrow = new Date(today);
  tomorrow.setDate(tomorrow.getDate() + 1);

  const result = await Transaction.aggregate([
    { $match: { type: 'sell', date: { $gte: today, $lt: tomorrow } } },
    { $group: { _id: null, total: { $sum: '$amount' } } }
  ]);

  const total = result.length > 0 ? result[0].total : 0;
  res.json({ total_sells: total });
});

// GET /api/buys
app.get('/api/buys', async (req, res) => {
  const today = new Date();
  today.setHours(0,0,0,0);
  const tomorrow = new Date(today);
  tomorrow.setDate(tomorrow.getDate() + 1);

  const result = await Transaction.aggregate([
    { $match: { type: 'buy', date: { $gte: today, $lt: tomorrow } } },
    { $group: { _id: null, total: { $sum: '$amount' } } }
  ]);

  const total = result.length > 0 ? result[0].total : 0;
  res.json({ total_buys: total });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));