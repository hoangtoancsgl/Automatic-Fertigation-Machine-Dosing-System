import logo from "../assets/logo.png";

const About = () => {
  return (
    <div className="About">
      <div className="navbar1">
        <img src={logo} className="logo" />
        <small className="universityTag">
          Ho Chi Minh
          <br />
          University of Technology
        </small>
        <a className="logintag" href="/login">
          Login
        </a>
      </div>
      <div class="content">
        <h1>
          Automatic Fertigation Machine
          <br />
          Dosing System for Hydroponic System
        </h1>
      </div>
    </div>
  );
};
export default About;
