import { useState } from "react";
import "./App.css";
import { Slider } from "@mui/material";

function App() {
  const [baseAngle, setBaseAngle] = useState(0);

  const handleBaseSliderChange = (event, newValue) => {
    setBaseAngle(newValue);
  };

  return (
    <>
      <h3>Base</h3>
      <Slider
        valueLabelDisplay="auto"
        value={baseAngle}
        onChange={handleBaseSliderChange}
        size="medium"
        sx={{ minWidth: "300px" }}
        min={0}
        max={360}
      />
      <h3>Elbow</h3>
      <Slider
        valueLabelDisplay="auto"
        size="medium"
        sx={{ minWidth: "300px" }}
        min={0}
        max={360}
      />
      <h3>Wrist</h3>
      <Slider
        valueLabelDisplay="auto"
        size="medium"
        sx={{ minWidth: "300px" }}
        min={0}
        max={360}
      />
      <h3>Claw</h3>
      <Slider
        valueLabelDisplay="auto"
        size="medium"
        sx={{ minWidth: "300px" }}
        min={0}
        max={180}
      />
    </>
  );
}

export default App;
