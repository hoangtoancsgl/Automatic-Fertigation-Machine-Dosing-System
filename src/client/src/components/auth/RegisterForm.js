import { Link } from "react-router-dom";
import { useState, useContext } from "react";
import { AuthContext } from "../../contexts/AuthContext";
import AlertMessage from "../layout/AlertMessage";
const Registerform = () => {
  const { registerUser } = useContext(AuthContext);

  //state
  const [registerForm, setRegisterForm] = useState({
    username: "",
    password: "",
    confirmPassword: "",
  });

  //Alert
  const [alert, setAlert] = useState(null);

  const { username, password, confirmPassword } = registerForm;

  const onChangeRegisterForm = (event) =>
    setRegisterForm({
      ...registerForm,
      [event.target.name]: event.target.value,
    });

  const register = async (event) => {
    event.preventDefault();

    if (password !== confirmPassword) {
      setAlert({ type: "danger", message: "Password do not match" });
      setTimeout(() => setAlert(null), 5000);
      return;
    }

    try {
      const registerData = await registerUser(registerForm);
      if (!registerData.success) {
        setAlert({ type: "danger", message: registerData.message });
        setTimeout(() => setAlert(null), 5000);
      }
    } catch (error) {
      console.log(error);
    }
  };
  return (
    <div className="signin">
      <h1>Register</h1>
      <form onSubmit={register}>
        <div className="txt_field">
          <input

            type="text"
            id="username"
            name="username"
            value={username}
            onChange={onChangeRegisterForm}
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
            onChange={onChangeRegisterForm}
            required
          />
          <span></span>
          <label>Password</label>
        </div>
        <div className="txt_field">
          <input
            className="input"
            type="password"
            id="Confirm Password"
            name="confirmPassword"
            value={confirmPassword}
            onChange={onChangeRegisterForm}
            required
          />
          <span></span>
          <label>Confirm Password</label>
        </div>
        <AlertMessage info={alert} />
        <input className="submit" type="submit" value="Register" />
        <p>
          Have an account?
          <Link to="login">
            <button variant="info" size="sm" className="ml-2">
              Login
            </button>
          </Link>
        </p>
      </form>
    </div>
  );
};

export default Registerform;
