import PostButtonTypeModal from "./PostButtonTypeModal";
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
      <div className="type">{typeModal}</div>
      <div className="TDS">{TDSModal}</div>
      <div className="dead_TDS">{dead_TDSModal}</div>
      <div className="PH">{PHModal}</div>
      <div className="dead_PH">{dead_PHModal}</div>
      <div className="nutri_ratio">{nutri_RatioModal}</div>
    </div>

    <PostButtonTypeModal
      _id={_id}
      type={typeModal}
      TDS={TDSModal}
      dead_TDS={dead_TDSModal}
      PH={PHModal}
      dead_PH={dead_PHModal}
      nutri_Ratio={nutri_RatioModal}
      className="actionButton"
    />
  </div>
);

export default SingleTypeModal;
