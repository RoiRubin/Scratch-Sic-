# Scratch(Sic) changelog

## 0.6
- Smooth or straight, one stretch of line at a time: right-click the small circle on a line and choose Smooth or Straight. A smooth stretch shows a circle, a straight one a diamond. A point added on a line keeps that line's character.
- The Smooth button in the toolbar now sets every stretch at once, and shows "on" only when all of them are smooth.
- Scratches saved before 0.6 keep their look: their single smooth setting is applied to every stretch.

## 0.5
- Undo / redo for everything in the plugin: Ctrl+Z, Ctrl+Y (or Ctrl+Shift+Z), and Undo / Redo buttons at the top. Covers the scratch graphs, the knobs, the Full sample and Smooth options, the sample window, and the loaded sample. A drag or a knob turn is one step. Up to 200 steps. Not covered: adding to or deleting from the bank, which are saved as files.
- Select several points: Shift + click toggles a point, Shift + drag draws a selection box (it adds to what is already selected). Dragging any selected point moves them all, keeping their shape. Right-click a selected point to remove them all.
- Hold Ctrl and drag either outer dot of a selection of two or more to tilt and stretch it as one line: the other outer dot stays put, and the dots between follow in proportion. A flat line becomes a diagonal, and a run can be stretched or squeezed in time.

## 0.4
- New "Full sample" option (on by default): Play and the export now include the whole sample, with the scratch in place of the scratched window and the untouched parts before and after it at normal speed. Switch it off to get the scratched part alone.
- Zoom for the graph: mouse wheel zooms around the pointer, Shift + wheel (or the scroll bar, or dragging the ruler) scrolls, and Fit (or a double-click on the ruler) shows everything again. Up to 100x. Finer snap steps (1/64, 1/128) for close-together scratches.
- A straight-up or straight-down line in the position graph is now a clean skip: the needle lands at the new spot at once instead of sweeping through the sample.
- The orange export tile has readable text that says what it does: drag it onto FL's playlist to drop the scratch in as audio.
- Exported file names show the total length, with "_full" when the whole sample is included.

## 0.3
- The graph now always covers the sample window. The Length dropdown is gone; the Window Length knob sets how long the scratch lasts.
- Bank scratches fill the whole graph. "Baby (slow)" is replaced by "Baby x2", since speed now comes from the window.
- The ruler shows seconds, with small orange ticks where FL's beats fall. The top bar shows the scratch length in seconds and in beats at FL's tempo.
- Snap is now a fraction of the graph (1/4 to 1/32).
- New "Smooth" option (on by default): the position line flows through the points like a hand, speeding up and slowing down inside each stroke and turning round gently at the peaks, instead of moving at constant speed.
- Fader cuts fade along a smooth S-curve instead of a straight ramp, and the new Fader Smooth knob sets how long each cut takes (0.5-20 ms, default 4 ms).
- Scratches saved with 0.2 are stretched to fill the graph when loaded.

## 0.2
- Scratching now happens inside the plugin. No automation clips needed.
- New scratching editor: a record position graph (points + bendable curves) over a fader graph (open blocks), with a beat ruler, snap, and a playhead.
- Scratch length in beats (1, 2, 3, 4, 8). Tempo is read from FL.
- Play / Loop preview inside the plugin.
- Scratch bank with six factory scratches (Baby, Baby slow, Chirp, Transformer, Flare, Scribble). Click one to load it, or drag it onto the waveform. "Add to bank" saves your own; they are stored in your app-data folder.
- "Drag into FL" tile: drag the scratch out as a 24-bit WAV onto the playlist, Channel Rack or Edison. WAVs are also kept in Documents\Scratch(Sic) Bounces.
- Choose the scratched part of the sample by dragging across the waveform.
- Sound: drawn curves are applied per sample (no more blocky steps), anti-aliasing for fast moves, click-free starts, and a Platter Weight knob for a heavier, more record-like feel.
- Removed: the automatable Record Position and Fader knobs.

## 0.1
- First version: a Channel Rack instrument that scratches a loaded sample.
- Automatable knobs: Record Position, Fader, Window Start, Window Length, Output.
- Record Position moves the needle inside the window picked by Window Start and Window Length.
- The record holds still (silent) whenever Record Position isn't moving.
- Load a sample with the button or by dropping a file on the waveform. The sample's path is saved with the FL project.
