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
    selectedDevice,
  },
}) => (
  <div className="singleType">
    <div className="value">
      <div className="typeSetting">{typeModal}</div>
      <div className="tds-ph-nutri">
        <div className="TDSSetting">
          <img src={TDSSet}></img>
          <div className="TDS_title"> TDS: {TDSModal}</div>
        </div>
        <div className="PHSetting">
          <img src={PHSet}></img>
          <div className="PH_title"> PH: {PHModal}</div>
        </div>
        <div className="nutri_ratioSetting">
          <img src={nutriRatioImg}></img>
          <div className="nutri_ratio_title">
            Nutri Ratio: {nutri_RatioModal}
          </div>
        </div>
      </div>
      <div className="dead-tds-ph">
        <div className="dead_TDSSetting">
          <img src={deadTDS}></img>
          <div className="dead_TDS_title">dead_TDS: {dead_TDSModal}</div>
        </div>
        <div className="dead_PHSetting">
          <img src={deadPHImg}></img>
          <div className="dead_PH_title"> dead_PH: {dead_PHModal}</div>
        </div>
      </div>
    </div>
    <PostButtonTypeModal _id={_id} selectedDevice={selectedDevice} />
  </div>
);

export default SingleTypeModal;
