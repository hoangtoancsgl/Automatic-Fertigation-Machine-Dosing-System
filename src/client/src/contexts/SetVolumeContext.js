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
          payload: responce.data.getLastConfigType[0],
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

  //   const deleteConfigType = async (configTypeId) => {
  //     try {
  //       const responce = await axios.delete(
  //         `${url}/configdata/configtype/${configTypeId}`
  //       );
  //       if (responce.data.success)
  //         dispatchType({ type: DELETE_CONFIGTYPE, payload: configTypeId });
  //     } catch (error) {
  //       console.log(error);
  //     }
  //   };

  // const findConfigType = (configTypeId) => {
  //   const configtype = typeState.config.find(
  //     (configtype) => configtype._id === configTypeId
  //   );
  //   dispatchType({ type: FIND_CONFIGTYPE, payload: configtype });
  // };

  // const updateConfigType = async (updatedConfigType) => {
  //   try {
  //     const response = await axios.put(
  //       `${url}/configdata/configtype/${updatedConfigType._id}`,
  //       updatedConfigType
  //     );
  //     if (response.data.success) {
  //       dispatchType({
  //         type: UPDATE_CONFIGTYPE,
  //         payload: response.data.post,
  //       });
  //       return response.data;
  //     }
  //   } catch (error) {
  //     return error.response.data
  //       ? error.response.data
  //       : { success: false, message: "Server error" };
  //   }
  // };

  const setVolumeContextData = {
    getSetVolume,
    addSetVolume,
    setVolumeState,
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
