const express = require('express')
const Tour = require('../models/tour')
const router = new express.Router()
router.post('/tours', (req, res) => {
    const tour = new Tour(req.body);
    tour.save().then(tour =>
        res.status(201).send(tour)
    ).catch(e => res.status(400).send(e))
})
router.delete('/tours/:id', (req, res) => {
        Tour.findOneAndDelete({ _id: req.params["id"] }, function (err) {
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

router.get('/tours', (req, res) => {
    Tour.find().then(tours => res.send(tours)
    ).catch(e => res.status(500).send(e))
})

router.get('/tours/:id', (req, res) => {
    const tourId = req.params["id"];
    Tour.findById(tourId, (err,docs) => {
        if (err) {
            return res
                .status(500)
                .send({ error: "unsuccessful to update tour" })
        }
        else{
            res.status(201).send(docs);
        }
    });
});
//UpdateTour
// Update Tour function that update trip by using request body parameters.
router.put('/tours/:id', (req, res) => {
    const tourId = req.params["id"].split("_")[0];
    var new_data = {start_date : "",price : "", duration: "",guide:{_id:""}};
    if(req.body.start_date) new_data.start_date = req.body.start_date;
    else delete(new_data.start_date)
    if(req.body.duration) new_data.duration = req.body.duration;
    else delete(new_data.duration)
    if(req.body.price) new_data.price = req.body.price;
    else delete(new_data.price)
    if(req.body.guide._id) new_data.guide._id = req.body.guide._id;
    else delete(new_data.guide)
    Tour.findByIdAndUpdate(tourId, new_data, (err) => {
        if (err) {
            return res
                .status(500)
                .send({ error: "unsuccessful to update tour" })
        }
        else{
            
            res.status(201).send({ success: "successful to update tour" });

        }
    });
});
//CreateSiteInTour
// Create Site In Path trip function that update just the path for the trip by using request body parameters.
router.put('/tours/site/:id', (req, res) => {
    const tourId = req.params["id"];
    Tour.exists({_id: tourId,path:{ name: req.body.path.name, country: req.body.path.country }}, 
        function (err, doc) {
            if (err || doc) {
                return res.status(500).send("unsuccessful to update site in tour")
            }
            else {
                Tour.findByIdAndUpdate({ _id: tourId },{$push: {'path':{ "name": req.body.path.name, "country": req.body.path.country }}},{ new: true }, 
                (err) => {
                        if (err) {
                            return res.status(500).send({ error: "unsuccessful to update site in tour" })
                        }
                        return res.status(201).send({ success: "successful to update site in tour" });
                    })
            }});
});
// DeleteSiteInTour
// Delete specific site in path by using request body parameters.
// country attr add because site name is not uniqe!!! 
router.delete('/tours/:id/:name/:country', (req, res) => {
    const tourId = req.params["id"];
    const tourName = req.params["name"];
    const tourCountry = req.params["country"];
    Tour.exists({
        _id: tourId,
        path:{ name: tourName, country: tourCountry }},
        function (err, doc) {
            if (tourName != "ALL" && tourCountry != "ALL") {
                Tour.updateOne({ _id: tourId }, { $pull: { path: { name: tourName, country: tourCountry } } }, function (err, docs) {
                    if (err) {
                        console.log(err)
                        return res.status(500).send({ error: "unsuccessful to delete site in tour" })
                    }
                    else {
                        return res.status(201).send({ success: "successful to delete site in tour" });
                    }
                });
            }
    });
})

module.exports = router