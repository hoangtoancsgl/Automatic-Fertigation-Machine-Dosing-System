import { createContext, useReducer, useState } from "react";
import { typeReducer } from "../reducers/typeReducer";
import { configReducer } from "../reducers/configReducer";
import {
  url,
  CONFIGTYPE_LOADED_SUCCESS,
  CONFIGTYPE_LOADED_FAIL,
  ADD_CONFIGTYPE,
  DELETE_CONFIGTYPE,
  FIND_CONFIGTYPE,
  UPDATE_CONFIGTYPE,
  ADD_CONFIGDATA,
  CONFIGDATA_LOADED_SUCCESS,
  CONFIGDATA_LOADED_FAIL,
} from "./constants";
import axios from "axios";
export const TypeContext = createContext();

const TypeContextProvider = ({ children }) => {
  const [typeState, dispatchType] = useReducer(typeReducer, {
    configtype: null,
    config: [],
    configLoading: true,
  });

  const [showAddTypeModal, setShowAddTypeModal] = useState(false);
  const [showUpdateTypeModal, setShowUpdateTypeModal] = useState(false);
  const [showToast, setShowToast] = useState({
    show: false,
    message: "",
    type: null,
  });

  /// get config type of Crops
  const getConfigType = async () => {
    try {
      const responce = await axios.get(`${url}/configdata/configtype`);
      if (responce.data.success) {
        dispatchType({
          type: CONFIGTYPE_LOADED_SUCCESS,
          payload: responce.data.getLastConfigType,
        });
      }
    } catch (error) {
      dispatchType({ type: CONFIGTYPE_LOADED_FAIL });
    }
  };

  const addConfigtype = async (newConfig) => {
    try {
      const response = await axios.post(
        `${url}/configdata/configtype`,
        newConfig
      );
      if (response.data.success) {
        dispatchType({
          type: ADD_CONFIGTYPE,
          payload: response.data.post,
        });
        return response.data;
      }
    } catch (error) {
      return error.response.data
        ? error.response.data
        : { success: false, message: "Server error" };
    }
  };

  const deleteConfigType = async (configTypeId) => {
    try {
      const responce = await axios.delete(
        `${url}/configdata/configtype/${configTypeId}`
      );
      if (responce.data.success)
        dispatchType({ type: DELETE_CONFIGTYPE, payload: configTypeId });
    } catch (error) {
      console.log(error);
    }
  };

  const findConfigType = (configTypeId) => {
    const configtype = typeState.config.find(
      (configtype) => configtype._id === configTypeId
    );
    dispatchType({ type: FIND_CONFIGTYPE, payload: configtype });
  };

  const updateConfigType = async (updatedConfigType) => {
    try {
      const response = await axios.put(
        `${url}/configdata/configtype/${updatedConfigType._id}`,
        updatedConfigType
      );
      if (response.data.success) {
        dispatchType({
          type: UPDATE_CONFIGTYPE,
          payload: response.data.post,
        });
        return response.data;
      }
    } catch (error) {
      return error.response.data
        ? error.response.data
        : { success: false, message: "Server error" };
    }
  };

  const typeContextData = {
    typeState,
    addConfigtype,
    getConfigType,
    showAddTypeModal,
    setShowAddTypeModal,
    showToast,
    setShowToast,
    deleteConfigType,
    findConfigType,
    updateConfigType,
    showUpdateTypeModal,
    setShowUpdateTypeModal,
  };
  return (
    <TypeContext.Provider value={typeContextData}>
      {children}
    </TypeContext.Provider>
  );
};
export default TypeContextProvider;
