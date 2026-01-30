const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');
const Transaction = require('./models/Transaction');

require('dotenv').config();

const app = express();

app.use(cors());
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

// GET /api/sells-summary
app.get('/api/sells-summary', async (req, res) => {
  const today = new Date();
  today.setHours(0,0,0,0);
  const tomorrow = new Date(today);
  tomorrow.setDate(tomorrow.getDate() + 1);

  const result = await Transaction.aggregate([
    { $match: { type: 'sell', date: { $gte: today, $lt: tomorrow } } },
    {
      $group: {
        _id: null,
        total_amount: { $sum: '$amount' },
        customer_count: { $addToSet: '$entity_id' }
      }
    },
    {
      $project: {
        total_amount: 1,
        customer_count: { $size: '$customer_count' }
      }
    }
  ]);

  const summary = result.length > 0 ? result[0] : { total_amount: 0, customer_count: 0 };
  res.json(summary);
});

// GET /api/buys-summary
app.get('/api/buys-summary', async (req, res) => {
  const today = new Date();
  today.setHours(0,0,0,0);
  const tomorrow = new Date(today);
  tomorrow.setDate(tomorrow.getDate() + 1);

  const result = await Transaction.aggregate([
    { $match: { type: 'buy', date: { $gte: today, $lt: tomorrow } } },
    {
      $group: {
        _id: null,
        total_amount: { $sum: '$amount' },
        product_count: { $addToSet: '$product_id' }
      }
    },
    {
      $project: {
        total_amount: 1,
        product_count: { $size: '$product_count' }
      }
    }
  ]);

  const summary = result.length > 0 ? result[0] : { total_amount: 0, product_count: 0 };
  res.json(summary);
});

// GET /api/revenue-summary
app.get('/api/revenue-summary', async (req, res) => {
  const today = new Date();
  today.setHours(0,0,0,0);
  const tomorrow = new Date(today);
  tomorrow.setDate(tomorrow.getDate() + 1);

  const sellsResult = await Transaction.aggregate([
    { $match: { type: 'sell', date: { $gte: today, $lt: tomorrow } } },
    {
      $group: {
        _id: '$payment',
        total: { $sum: '$amount' }
      }
    }
  ]);

  const buysResult = await Transaction.aggregate([
    { $match: { type: 'buy', date: { $gte: today, $lt: tomorrow } } },
    {
      $group: {
        _id: '$payment',
        total: { $sum: '$amount' }
      }
    }
  ]);

  let total_revenue = 0;
  let due_to_customers = 0;
  sellsResult.forEach(item => {
    total_revenue += item.total;
    if (item._id === 'due') due_to_customers += item.total;
  });

  let due_from_suppliers = 0;
  buysResult.forEach(item => {
    if (item._id === 'due') due_from_suppliers += item.total;
  });

  res.json({ total_revenue, due_to_customers, due_from_suppliers });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});