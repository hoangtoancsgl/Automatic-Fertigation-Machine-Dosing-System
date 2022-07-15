const express = require('express');
const mongoose = require('mongoose');
const Blog = require('./views/blog.js');

// connect to mongoDB
const dbURL = 'mongodb+srv://admin:admin@dclv.zr9nw.mongodb.net/myFirstDatabase?retryWrites=true&w=majority';
mongoose.connect(dbURL, {useNewUrlParser: true, useUnifiedTopology: true })
    .then((result) => app.listen(3000) )
    .catch((err) => console.log (err));

const app = express();

app.set( 'view engine' , 'ejs' );

//listen  to request

//middleware && static files
app.use(express.static('views/static/css'));
app.use(express.urlencoded({extended:true}));

app.get('/add-blog',(req,res)=>{
    const blog = new Blog({
        text: 'admin',
        password: 'admin'
    });
    blog.save()
        .then((result) =>{
            res.send(result)
        })
        .catch((err) =>{
            console.log(err);
        })
})

app.get('/', (req, res) => {                                                                                   //Homepage
   res.render('index' );
});

app.get( '/signinpage' , ( req, res ) => {                                                              //Sign in Page
   res.render('signin');
});

app.get( '/mainpage' , ( req, res ) => {                                                              //Main Page
    res.render('mainclient');
 });

//app.use( ( req, res ) => {                                                                                   //404
//    res.status('404').render('404');
// });
app.get('/signin',(req,res) =>{
    Blog.find()
    .then((result) =>{
        res.send(result);
        console.log(req.body);
        console.log(res.body);
    })
    .catch((err) =>{
        console.log(err);
    })
});
/*
 app.post('/signinpage' ,(req,res) =>{
    console.log(req.body);
    blog = new Blog(req.body);
    blog.save()
    .then((result) =>{
        res.redirect('/mainpage');
    })
    .catch((err) =>{
        console.log(err);
    })
 })
 */