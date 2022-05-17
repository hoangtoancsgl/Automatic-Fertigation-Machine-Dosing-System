import { createContext, useReducer } from "react";
import { statusReducer } from "../reducers/statusReducer";
import { url, STATUS_LOADED_FAIL, STATUS_LOADED_SUCCESS } from "./constants";
import axios from "axios";
export const StatusContext = createContext();

const StatusContextProvider = ({ children }) => {
  const [statusState, dispatchStatus] = useReducer(statusReducer, {
    status: [],
    statusLoading: true,
  });
  /// get status of device
  const getStatus = async (device) => {
    try {
      const responce = await axios.get(`${url}/state/${device}`);
      if (responce.data.success) {
        dispatchStatus({
          type: STATUS_LOADED_SUCCESS,
          payload: responce.data.getState,
        });
      }
    } catch (error) {
      dispatchStatus({ type: STATUS_LOADED_FAIL });
    }
  };

  const statusContextData = {
    statusState,
    getStatus,
  };
  return (
    <StatusContext.Provider value={statusContextData}>
      {children}
    </StatusContext.Provider>
  );
};
export default StatusContextProvider;
