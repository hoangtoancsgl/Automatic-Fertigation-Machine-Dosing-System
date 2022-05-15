import React, { PureComponent, Fragment } from "react";
import ReactDOM from "react-dom";

class Select extends PureComponent {
  state = {
    options: [
      {
        name: "Select…",
        value: null,
      },
      {
        name: "A",
        value: "a",
      },
      {
        name: "B",
        value: "b",
      },
      {
        name: "C",
        value: "c",
      },
    ],
    value: "?",
  };

  handleChange = (event) => {
    this.setState({ value: event.target.value });
  };

  render() {
    const { options, value } = this.state;

    return (
      <Fragment>
        <select onChange={this.handleChange} value={value}>
          {options.map((item) => (
            <option key={item.value} value={item.value}>
              {item.name}
            </option>
          ))}
        </select>
        <h1>Favorite letter: {value}</h1>
      </Fragment>
    );
  }
}

ReactDOM.render(<Select />, window.document.body);
