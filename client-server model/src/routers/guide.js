const express = require('express')
const Guide = require('../models/guide')
const router = new express.Router()

router.post('/guides', (req, res) => {
    const guide = new Guide(req.body);

    guide.save().then(guide=>
        res.status(201).send(guide)
    ).catch(e=>res.status(400).send(e))
})

router.get('/guides', (req, res) => {
    Guide.find().populate('author').then(guide => res.send(guide)
    ).catch (e=> res.status(500).send())
})
router.get('/guides/:id', (req, res) => {
    Guide.findById(req.params["id"], function (err, docs) {
        if (err){
            console.log(err);
        }
        else{
            res.status(201).send(docs);
        }
    });
})
router.put('/guides/:id', (req, res) => {
    const guideID = req.params["id"].split("_")[0];
    var new_data = {name : "",email : "", cellular: ""};
    if(req.body.name) new_data.name = req.body.name;
    else delete(new_data.name)
    if(req.body.email) new_data.email = req.body.email;
    else delete(new_data.email)
    if(req.body.cellular) new_data.cellular = req.body.cellular;
    else delete(new_data.cellular)
    Guide.findByIdAndUpdate(guideID, new_data, (err) => {
        if (err) {
            return res
                .status(500)
                .send({ error: "unsuccessful to update guide" })
        }
        else{
            
            res.status(201).send({ success: "successful to update guide" });

        }
    });
});

router.delete('/guides/:id', (req, res) => {
    Guide.findOneAndDelete({ _id: req.params["id"] }, function (err) {
    if (err) {
        return res
            .status(500)
            .send({ error: "unsuccessful to delete site in tour" })
    }
    else {
        return res.status(201).send({ success: "successful to delete site in tour" });
    }
});
})

module.exports = router