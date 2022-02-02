path = require('path')
fs = require('fs')
const express = require('express')
require('./db/mongoose')
const guideRouter = require('./routers/guide')
const tourRouter = require('./routers/tour')

const app = express()
const port = process.env.PORT || 3000
app.use('/update_tour', express.static(path.join(__dirname, 'html/update_tour.html')));
app.use('/create_tour', express.static(path.join(__dirname, 'html/create_tour.html')));
app.use('/create_guide', express.static(path.join(__dirname, 'html/create_guide.html')));
app.use('/add_guide', express.static(path.join(__dirname, 'html/add_guide_form.html')));
app.use('/js', express.static(path.join(__dirname, 'js')));
app.get('/', (req, res) => {
    fs.readFile('src/html/index.html', (err, html) => {
        if (err) {
            throw err;
        }
        res.writeHeader(200, { "Content-Type": "text/html" });
        res.write(html);
        res.end();
    })
});

app.use('/public', express.static(path.join(__dirname, 'public')));
app.use('/html', express.static(path.join(__dirname, 'html')));

// app.use(cors());
app.use(express.urlencoded({ extended: true }));

app.use(express.json())
app.use(guideRouter)
app.use(tourRouter)

app.listen(port, () => {
    console.log('Server is up on port ' + port)
})