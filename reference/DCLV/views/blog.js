const mongoose =require('mongoose');
const Schema = mongoose.Schema;

const blogSchema = new Schema({
    text:{
        type: String,
        required: true
    },
    password:{
        type: String,
        required: true
    }

}, { timestamps:true });
const Blog = mongoose.model('Blog',blogSchema);
module.exports =Blog;