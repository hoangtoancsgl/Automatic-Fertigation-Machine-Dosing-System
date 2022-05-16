import deleteIcon from "../assets/deleteButton.jpeg";
import editIcon from "../assets/editButton.png";
import Tooltip from "react-bootstrap/Tooltip";
import selectButton from "../assets/selectButton.png";
import OverlayTrigger from "react-bootstrap/OverlayTrigger";
import { TypeContext } from "../contexts/TypeContext";
import { useContext, useEffect, useState } from "react";
import { DataContext } from "../contexts/DataContext";
import { ConfigContext } from "../contexts/ConfigContext";

const ActionButtons = ({ _id }) => {
  const {
    dataState: {
      data: { device },
    },
  } = useContext(DataContext);
  const {
    typeState: { configtype },
    updateConfigType,
  } = useContext(TypeContext);

  const { configManual } = useContext(ConfigContext);

  const { deleteConfigType, findConfigType, setShowUpdateTypeModal } =
    useContext(TypeContext);

  const chooseType = (configTypeId) => {
    findConfigType(configTypeId);
    setShowUpdateTypeModal(true);
  };

  const choosePostType = (configTypeId) => {
    findConfigType(configTypeId);
  };

  const { setShowToast } = useContext(TypeContext);
  const onClickConf = async (event) => {
    event.preventDefault();
    console.log(configtype.type, configtype.TDS);
    try {
      const { success, message } = await configManual({
        device: device,
        type: configtype.type,
        TDS: configtype.TDS,
        dead_TDS: configtype.dead_TDS,
        PH: configtype.PH,
        dead_PH: configtype.dead_PH,
        nutri_Ratio: configtype.nutri_Ratio,
      });
      setShowToast({ show: true, message, Type: success ? "info" : "danger" });
    } catch (error) {
      console.log(error);
    }
  };

  return (
    <div className="actionButton">
      <OverlayTrigger
        placement="left"
        overlay={<Tooltip>Choose this type for current config</Tooltip>}
      >
        <form onSubmit={onClickConf}>
          <button
            type="submit"
            onClick={choosePostType.bind(this, _id)}
            className="inputtype-button"
          >
            <img src={selectButton} width="30" height="30" />
          </button>
        </form>
      </OverlayTrigger>
      <OverlayTrigger
        placement="left"
        overlay={<Tooltip>Change config</Tooltip>}
      >
        <button className="type-button" onClick={chooseType.bind(this, _id)}>
          <img src={editIcon} width="30" height="30" />
        </button>
      </OverlayTrigger>

      <OverlayTrigger
        placement="left"
        overlay={<Tooltip>Delete config </Tooltip>}
      >
        <button
          className="type-button"
          onClick={deleteConfigType.bind(this, _id)}
        >
          <img src={deleteIcon} width="30" height="30" />
        </button>
      </OverlayTrigger>
    </div>
  );
};
export default ActionButtons;
