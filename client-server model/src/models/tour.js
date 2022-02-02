const mongoose = require('mongoose');
const id_validator = require('mongoose-id-validator');

var TourSchema = new mongoose.Schema({
    name: {
        type: String,
        required: true,
        trim: true
    },
    start_date: {
        type: Date,
        required: true,
    },
    duration: {
        type: Number,
        required: true,
        min: 1,
        default: 1
    },
    price: {
        type: Number,
        required: true,
        min: 1,
        default: 1
    },
    guide: {
        type: mongoose.Schema.Types.ObjectId, ref: 'Guide',
        required: true
    },
    path: { "type": ["Mixed"] }
}, { timestamps: true });
TourSchema.plugin(id_validator);
TourSchema.index("completed");

const Tour = mongoose.model('tour', TourSchema);

module.exports = Tour