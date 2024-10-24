import {useEffect, useRef} from 'react';
import {ControlsContainer} from './ControlsContainer';
import {ControlSlider} from './ControlSlider';
import {Gui} from '../dat.gui/Gui';
import {ControlComponent} from '../dat.gui/types';
import {ControlSwitch} from './ControlSwitch';
import {ControlOptions} from './ControlOptions';
import {ControlTextInput} from './ControlTextInput';
import {ControlButton} from './ControlButton';
import {ControlFolder} from './ControlFolder';
import {ControlNumberInput} from './ControlNumberInput';

export const useControls = () => {
  const gui = useRef(new Gui()).current;
  const Controls = () => {
    const renderResult = gui.observable.useState();
    useEffect(() => {
      return () => gui.destroy();
    }, []);
    const renderComponent = (component: ControlComponent) => {
      if (component.type === 'function') {
        return <ControlButton {...component.props} />;
      }
      if (component.type === 'folder') {
        const children = component.props.children.map(renderComponent);
        return component.props.isRoot ? (
          children
        ) : (
          <ControlFolder {...component.props}>{children}</ControlFolder>
        );
      }
      if (component.type === 'slider') {
        return <ControlSlider {...component.props} />;
      }
      if (component.type === 'number') {
        return <ControlNumberInput {...component.props} />;
      }
      if (component.type === 'switch') {
        return <ControlSwitch {...component.props} />;
      }
      if (component.type === 'options') {
        return <ControlOptions {...component.props} />;
      }
      if (component.type === 'string') {
        return <ControlTextInput {...component.props} />;
      }
      return null;
    };
    return renderResult ? (
      <ControlsContainer>{renderComponent(renderResult)}</ControlsContainer>
    ) : null;
  };
  Controls.displayName = 'Controls';
  return {
    gui,
    Controls,
  };
};
