import { createContext, useReducer, useEffect } from "react";
import axios from "axios";
import { url } from "./constants";
import { LOCAL_STORAGE_TOKEN_NAME } from "./constants";
import { authReducer } from "../reducers/authReducer";
import setAuthToken from "../utils/setAuthToken";

export const AuthContext = createContext();

const AuthContextProvider = ({ children }) => {
  const [authState, dispatch] = useReducer(authReducer, {
    authLoading: true,
    isAuthenticated: false,
    user: null,
  });

  //authe user
  const loadUser = async () => {
    if (localStorage[LOCAL_STORAGE_TOKEN_NAME]) {
      setAuthToken(localStorage[LOCAL_STORAGE_TOKEN_NAME]);
    }
    try {
      const responce = await axios.get(`${url}/auth`);
      dispatch({
        type: "SET_AUTH",
        payload: { isAuthenticated: true, user: responce.data.user },
      });
    } catch (error) {
      localStorage.removeItem(LOCAL_STORAGE_TOKEN_NAME);
      setAuthToken(null);
      dispatch({
        type: "SET_AUTH",
        payload: { isAuthenticated: false, user: null },
      });
    }
  };

  //
  useEffect(() => loadUser(), []);

  //register
  const registerUser = async (userForm) => {
    try {
      const response = await axios.post(`${url}/auth/register`, userForm);
      if (response.data.success) {
        localStorage.setItem(
          LOCAL_STORAGE_TOKEN_NAME,
          response.data.accessToken
        );
        await loadUser();
        return response.data;
      }
    } catch (error) {
      if (error.response.data) return error.response.data;
      else return { success: false, message: error.message.data };
    }
  };

  //login
  const loginUser = async (userForm) => {
    try {
      const response = await axios.post(`${url}/auth/login`, userForm);
      if (response.data.success) {
        localStorage.setItem(
          LOCAL_STORAGE_TOKEN_NAME,
          response.data.accessToken
        );
        await loadUser();
        return response.data;
      }
    } catch (error) {
      if (error.response.data) return error.response.data;
      else return { success: false, message: error.message.data };
    }
  };
  //logout
  const logoutUser = () => {
    localStorage.removeItem(LOCAL_STORAGE_TOKEN_NAME);
    dispatch({
      type: "SET_AUTH",
      payload: { isAuthenticated: false, user: null },
    });
  };

  //addDevice
  

  //data context
  const AuthContextData = { loginUser, registerUser, logoutUser, authState };
  return (
    <AuthContext.Provider value={AuthContextData}>
      {children}
    </AuthContext.Provider>
  );
};
export default AuthContextProvider;
