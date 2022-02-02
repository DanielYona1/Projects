const mongoose = require('mongoose')



async function main() {
    var uri = 'mongodb://127.0.0.1:27017/shuru';
    // var uri = "mongodb+srv://yarin:131071@cluster0.iz2ji.mongodb.net/shuru"
    try{
        mongoose.connect(uri, {
            useNewUrlParser: true,
            useCreateIndex: true,
            useFindAndModify: false,
            useUnifiedTopology: true
        });

    }catch(err){
        console.log("error in main",err);
    }
}
main();