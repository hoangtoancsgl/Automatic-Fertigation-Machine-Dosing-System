import { Link } from "react-router-dom";
import { useState, useContext } from "react";
import { AuthContext } from "../../contexts/AuthContext";
import AlertMessage from "../layout/AlertMessage";

const LoginForm = () => {
  //context
  const { loginUser } = useContext(AuthContext);

  //state
  const [loginForm, setLoginForm] = useState({
    username: "",
    password: "",
  });

  //Alert
  const [alert, setAlert] = useState(null);

  const { username, password } = loginForm;

  const onChangeLoginForm = (event) =>
    setLoginForm({ ...loginForm, [event.target.name]: event.target.value });

  const login = async (event) => {
    event.preventDefault();

    try {
      const loginData = await loginUser(loginForm);
      if (!loginData.success) {
        setAlert({ type: "danger", message: loginData.message });
        setTimeout(() => setAlert(null), 5000);
      }
    } catch (error) {
      console.log(error);
    }
  };
  return (
    <div className="signin">
      <h1>Login</h1>
      <form className="my-4" onSubmit={login}>
        <div className="txt_field">
          <input
            type="text"
            id="username"
            name="username"
            value={username}
            onChange={onChangeLoginForm}
            required
          />
          <span></span>
          <label>Username</label>
        </div>
        <div className="txt_field">
          <input
            type="password"
            id="password"
            name="password"
            value={password}
            onChange={onChangeLoginForm}
            required
          />
          <span></span>
          <label>Password</label>
        </div>
        <AlertMessage info={alert} />
        <input className="submit" type="submit" value="Login" />
        <p>
          Don't have an account?
          <Link to="register">
            <button variant="info" size="sm" className="ml-2">
              Register
            </button>
          </Link>
        </p>
      </form>
    </div>
  );
};
export default LoginForm;
