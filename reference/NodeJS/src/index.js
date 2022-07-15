const path = require('path');

const  express = require('express');
const  handlebars  = require('express-handlebars');
const morgan  = require('morgan');

const  app = express();

app.use(express.static(path.join(__dirname, 'public')));
//HTTP loger
// app.use(morgan('combined'));

app.use(express.urlencoded({
    extended: true
}));
//Template engine
app.engine('hbs', handlebars({
    extname: '.hbs'
}));
app.set('view engine', 'hbs');
app.set('views', path.join(__dirname, 'resources', 'views'));


app.get('/', function (req, res) {
    res.render('home');
});

app.get('/search', function (req, res) {
    res.render('search');
});

app.post('/search', function (req, res) {
    console.log(req.body);
    res.send('search');
});

app.listen(3000);

