const express = require('express');
const fs = require('fs');
const bodyParser = require('body-parser');
const cors = require('cors');
const { Sequelize, DataTypes, Model } = require('sequelize');

const PORT = 3000;
const app = express();

app.use(cors());
app.use(bodyParser.json({ limit: '50mb', extended: true }));

app.use('/api/dumps', express.static('dumps'))

const sequelize = new Sequelize({
    dialect: 'sqlite',
    storage: 'db/threats.sqlite'
});

class Threat extends Model {
    json() {
        return {
            id: this.id,
            name: this.name,
            hash: this.hash,
            type: this.type,
            description: this.description,
            date: this.date
        }
    }
}

Threat.init({
    name: {
        type: DataTypes.STRING,
        allowNull: false
    },
    hash: {
        type: DataTypes.STRING,
        allowNull: false
    },
    type: {
        type: DataTypes.STRING,
        allowNull: false
    },
    description: {
        type: DataTypes.STRING
    },
    date: {
        type: DataTypes.DATE,
        defaultValue: Sequelize.NOW
    }
}, {
    sequelize,
    modelName: 'Threat'
});

async function handleGetThreats(req, res, next) {
    try {
        const threats = await Threat.findAll();
        const json = threats.map(threat => threat.json());
        res.status(200).json(json);
    } catch (e) {
        next(e);
    }
}

async function handleGetThreat(req, res, next) {
    try {
        const { id } = req.params;
        const entry = await Threat.findByPk(id);
        res.status(200).json(entry.json());
    } catch(e) {
        next(e);
    }
}

async function handlePostThreat(req, res, next) {
    try {
        const { name, hash, type, description, dump } = req.body;
        
        fs.writeFile(`dumps/${hash}.bin`, dump, 'base64', err => {});

        const entry = await Threat.create({
            name,
            hash,
            type,
            description
        });

        res.status(200).json(entry.json());
    } catch (e) {
        next(e);
    }
}

app.get('/api/threats', handleGetThreats);
app.get('/api/threats/:id', handleGetThreat);
app.post('/api/threats', handlePostThreat);

async function run() {
    await sequelize.sync();
    
    app.listen(PORT, () => {
        console.log(`Listening on port ${PORT}`)
    });
}

run();
