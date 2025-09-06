// signer/index.js
// npm init -y; npm install express ethers dotenv body-parser
require('dotenv').config();
const express = require('express');
const bodyParser = require('body-parser');
const { ethers } = require('ethers');
const app = express(); app.use(bodyParser.json());
if(!process.env.PRIVATE_KEY || !process.env.RPC_URL) { console.error('Set PRIVATE_KEY and RPC_URL in .env'); process.exit(1); }
const provider = new ethers.JsonRpcProvider(process.env.RPC_URL);
const wallet = new ethers.Wallet(process.env.PRIVATE_KEY, provider);
app.post('/send', async (req, res) => {
  try {
    const { to, data, value } = req.body;
    const tx = await wallet.sendTransaction({ to, data, value: value || '0x0' });
    res.json({ txHash: tx.hash });
  } catch(e) { res.status(500).json({ error: e.message }); }
});
const port = process.env.PORT || 3000; app.listen(port, ()=>console.log('Signer listening', port));