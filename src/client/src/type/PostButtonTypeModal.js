import Tooltip from "react-bootstrap/Tooltip";
import selectButton from "../assets/selectButton.png";
import OverlayTrigger from "react-bootstrap/OverlayTrigger";
import { TypeContext } from "../contexts/TypeContext";
import { useContext } from "react";
import { DataContext } from "../contexts/DataContext";
import { ConfigContext } from "../contexts/ConfigContext";
import { TypeModalContext } from "../contexts/TypeModalContext";
const PostButtonTypeModal = ({ _id }) => {
  const {
    dataState: {
      data: { device },
    },
  } = useContext(DataContext);
  const {
    typeModalState: { configtypemodal },
  } = useContext(TypeModalContext);

  const { setShowToast } = useContext(TypeContext);

  console.log(_id);
  const { config } = useContext(ConfigContext);
  const { findConfigTypeModal } = useContext(TypeModalContext);
  const choosePostType = (configTypeId) => {
    console.log(configTypeId);
    findConfigTypeModal(configTypeId);
  };
  const onClickConf = async (event) => {
    event.preventDefault();
    try {
      const { success, message } = await config({
        device: device,
        type: configtypemodal.typeModal,
        TDS: configtypemodal.TDSModal,
        dead_TDS: configtypemodal.dead_TDSModal,
        PH: configtypemodal.PHModal,
        dead_PH: configtypemodal.dead_PHModal,
        nutri_Ratio: configtypemodal.nutri_RatioModal,
      });
      setShowToast({ show: true, message, Type: success ? "info" : "danger" });
    } catch (error) {
      console.log(error);
    }
  };

  return (
    <div>
      <OverlayTrigger
        placement="left"
        overlay={<Tooltip>Choose this type for current config</Tooltip>}
      >
        <form onSubmit={onClickConf}>
          <button type="submit" onClick={choosePostType.bind(this, _id)}>
            <img src={selectButton} width="30" height="30" />
          </button>
        </form>
      </OverlayTrigger>
    </div>
  );
};
export default PostButtonTypeModal;
