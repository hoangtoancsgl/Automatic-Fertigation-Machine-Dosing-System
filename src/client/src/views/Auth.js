import LoginForm from "../components/auth/LoginForm";
import RegisterForm from "../components/auth/RegisterForm";
import logo from "../assets/logo.png";
import { AuthContext } from "../contexts/AuthContext";
import { useContext } from "react";
import { Redirect } from "react-router-dom";
import Spinner from "react-bootstrap/esm/Spinner";

const Auth = ({ authRoute }) => {
  const {
    authState: { authLoading, isAuthenticated },
  } = useContext(AuthContext);

  let body;

  if (authLoading)
    body = (
      <div className="d-flex justify-content-center mt-2">
        <Spinner animation="border" variant="info" />
      </div>
    );
  else if (isAuthenticated) return <Redirect to="/dashboard" />;
  else
    body = (
      <>
        {authRoute === "login" && <LoginForm />}
        {authRoute === "register" && <RegisterForm />}
      </>
    );
  return (
    <div className="Home">
      <div className="navbar">
        <img src={logo} className="logo" />
        <small className="universityTag">
          Ho Chi Minh
          <br />
          University of Technology
        </small>
      </div>
      <div className="signin">
        <h2></h2>
        {body}
      </div>
    </div>
  );
};
export default Auth;
