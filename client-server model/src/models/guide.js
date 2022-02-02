const mongoose = require('mongoose')
const validator = require('validator')

var GuideSchema = new mongoose.Schema({
    name: {
        type: String,
        required: true,
        trim: true
    },
    email: {
        type: String,
        required: true,
        trim: true,
        lowercase: true,
        validate(value) {
            if (!validator.isEmail(value)) {
                throw new Error('Email is invalid')
            }
        }
    },
    cellular: {
        type: String,
        required: true,
        minlength: 10,
        trim: true,
        validate(value) {
            if (value.toLowerCase().includes("/^[\+]?[(]?[0-9]{3}[)]?[-\s\.]?[0-9]{3}[-\s\.]?[0-9]{4,6}$/im")) {
                throw new Error('cellular is invalid')
            }
        }
    },
}, { timestamps: true }
);

const Guide = mongoose.model('Guide', GuideSchema);

module.exports = Guide