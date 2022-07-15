var express = require("express");
var app = express();


app.set("view engine", "ejs");
app.set("views", "/.views");
app.set(express.static("public"));

const mongoose = require("mongoose");
mongoose.connect('mongodb+srv://hoangtoancsgl:bX5fGVmtzwG406li@cluster0.jpi0o.mongodb.net/Dosing_System?retryWrites=true&w=majority', function(err){
    if(!err){
        console.log("MongoDB connected!");
    }
    else{
        console.log("MongoDB err!");
    }
});


const Cap1 = require("./models/cap1");
const Cap2 = require("./models/cap2");

app.get("/cap1/:name", function(req, res){

    var cap1 = new Cap1({
        name: req.params.name,
        kids: []
    });

    cap1.save(function(err){
        if(!err){
            res.json({kq:1});
        }
        else{
            res.json({kq:0});
        }
    });
});

app.get("/cap2/:IDme/:name", function(req, res){

    var cap2 = new Cap2({
        name: req.params.name
    });

    cap2.save(function(err){
        if(!err){
            Cap1.findOneAndUpdate({_id: req.params.IDme}, {$push: {kids: cap2._id}}, function(err){
                if(!err){
                    res.json({kq:1});
                }
                else{
                    res.json({kq:0});
                }
            });
        }
        else{
            res.json({kq:0});
        }
    });
});

app.get("/menu", function(req, res){
    var cap1 = Cap1.aggregate([{
        $lookup: {
            from: "cap2",
            localField: "kids",
            foreignField: "_id",
            as: "Con"
        }
    }], function(err, data){
        if(!err){
            res.json(data);
        }
        else{
            res.json({kq:0});
        }
    });
});
app.listen(3000);

app.get("/", function(req, res){
    res.send("OK");
})

//bX5fGVmtzwG406li