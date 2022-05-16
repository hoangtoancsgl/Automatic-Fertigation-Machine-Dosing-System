import { createContext, useReducer, useState } from "react";
import { setVolumeReducer } from "../reducers/setVolumeReducer";
import {
  url,
  SETVOLUME_LOADED_FAIL,
  SETVOLUME_LOADED_SUCCESS,
  ADD_SETVOLUME,
  FIND_SETVOLUME,
  UPDATE_SETVOLUME,
} from "./constants";
import axios from "axios";
export const SetVolumeContext = createContext();

const SetVolumeContextProvider = ({ children }) => {
  const [setVolumeState, dispatchSetVolume] = useReducer(setVolumeReducer, {
    //configtype: null,
    setVolume: [],
    setVolumeLoading: true,
  });

  //   const [showAddTypeModal, setShowAddTypeModal] = useState(false);
  //   const [showUpdateTypeModal, setShowUpdateTypeModal] = useState(false);
  //   const [showToast, setShowToast] = useState({
  //     show: false,
  //     message: "",
  //     type: null,
  //   });

  /// get config type of Crops
  const getSetVolume = async () => {
    try {
      const responce = await axios.get(`${url}/setvolume`);
      if (responce.data.success) {
        dispatchSetVolume({
          type: SETVOLUME_LOADED_SUCCESS,
          payload: responce.data.getLastConfigType,
        });
      }
    } catch (error) {
      dispatchSetVolume({ type: SETVOLUME_LOADED_FAIL });
    }
  };

  const addSetVolume = async (newConfig) => {
    try {
      const response = await axios.post(`${url}/setvolume`, newConfig);
      if (response.data.success) {
        dispatchSetVolume({
          type: ADD_SETVOLUME,
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

  const updateVolume = async (updatedVolume) => {
    try {
      const response = await axios.put(
        `${url}/setvolume/${updatedVolume._id}`,
        updatedVolume
      );
      if (response.data.success) {
        dispatchSetVolume({
          type: UPDATE_SETVOLUME,
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

  const setVolumeContextData = {
    getSetVolume,
    addSetVolume,
    setVolumeState,
    updateVolume,
    // getConfigType,
    // showAddTypeModal,
    // setShowAddTypeModal,
    // showToast,
    // setShowToast,
    // deleteConfigType,
    // findConfigType,
    // updateConfigType,
    // showUpdateTypeModal,
    // setShowUpdateTypeModal,
  };
  return (
    <SetVolumeContext.Provider value={setVolumeContextData}>
      {children}
    </SetVolumeContext.Provider>
  );
};
export default SetVolumeContextProvider;
