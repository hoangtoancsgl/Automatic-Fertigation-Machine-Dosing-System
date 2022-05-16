import PostButtonTypeModal from "./PostButtonTypeModal";
import nutriRatioImg from "../assets/nutriRatio.png";
import deadPHImg from "../assets/PH_dead.png";
import PHSet from "../assets/PHSet.png";
import TDSSet from "../assets/TDS_img.png";
import deadTDS from "../assets/TDS-deadvalue.png";
import "../views/views.css";
const SingleTypeModal = ({
  typeModal: {
    _id,
    typeModal,
    TDSModal,
    dead_TDSModal,
    PHModal,
    dead_PHModal,
    nutri_RatioModal,
  },
}) => (
  <div className="singleType">
    <div className="value">
      <div className="typeSetting">{typeModal}</div>
      <div className="tds-ph-nutri">
        <div className="TDSSetting">
          <img src={TDSSet}></img>
          TDS: {TDSModal}
        </div>
        <div className="PHSetting">
          <img src={PHSet}></img>
          PH: {PHModal}
        </div>
        <div className="nutri_ratioSetting">
          <img src={nutriRatioImg}></img>
          Nutri Ratio: {nutri_RatioModal}
        </div>
      </div>
      <div className="dead-tds-ph">
        <div className="dead_TDSSetting">
          <img src={deadTDS}></img>
          dead_TDS: {dead_TDSModal}
        </div>
        <div className="dead_PHSetting">
          <img src={deadPHImg}></img>
          dead_PH: {dead_PHModal}
        </div>
      </div>
    </div>
    <PostButtonTypeModal
      _id={_id}
      type={typeModal}
      TDS={TDSModal}
      dead_TDS={dead_TDSModal}
      PH={PHModal}
      dead_PH={dead_PHModal}
      nutri_Ratio={nutri_RatioModal}
    />
  </div>
);

export default SingleTypeModal;
