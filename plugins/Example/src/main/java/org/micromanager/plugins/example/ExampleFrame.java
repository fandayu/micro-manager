/**
 * ExampleFrame.java
 *
 * This module shows an example of creating a GUI (Graphical User Interface).
 * There are many ways to do this in Java; this particular example uses the
 * MigLayout layout manager, which has extensive documentation online.
 *
 *
 * Nico Stuurman, copyright UCSF, 2012, 2015
 *
 * LICENSE: This file is distributed under the BSD license. License text is
 * included with the source distribution.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
 */
package org.micromanager.example;

import com.google.common.eventbus.Subscribe;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.Font;
import java.util.List;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

import mmcorej.CMMCore;

import net.miginfocom.swing.MigLayout;

import org.micromanager.data.Image;
import org.micromanager.display.HistogramData;
import org.micromanager.events.ExposureChangedEvent;
import org.micromanager.Studio;
import org.micromanager.internal.utils.MMFrame;

public class ExampleFrame extends MMFrame {

   private Studio studio_;
   private JTextField userText_;
   private JLabel imageInfoLabel_;
   private JLabel exposureTimeLabel_;

   public ExampleFrame(Studio studio) {
      super("Example Plugin GUI");
      studio_ = studio;

      setLayout(new MigLayout("fill, insets 2, gap 2, flowx"));

      JLabel title = new JLabel("I'm an example plugin!");
      title.setFont(new Font("Arial", Font.BOLD, 14));
      add(title, "span, alignx center, wrap");

      // Create a text field for the user to customize their alerts.
      add(new JLabel("Alert text: "));
      userText_ = new JTextField(30);
      userText_.setText("Something happened!");
      add(userText_);

      JButton alertButton = new JButton("Alert me!");
      // Clicking on this button will invoke the ActionListener, which in turn
      // will show a text alert to the user.
      alertButton.addActionListener(new ActionListener() {
         @Override
         public void actionPerformed(ActionEvent e) {
            // Use the contents of userText_ as the text.
            studio_.alerts().postAlert("Example Alert!",
               ExampleFrame.class, userText_.getText());
         }
      });
      add(alertButton, "wrap");

      // Snap an image, show the image in the Snap/Live view, and show some
      // stats on the image in our frame.
      imageInfoLabel_ = new JLabel();
      add(imageInfoLabel_, "growx, split, span");
      JButton snapButton = new JButton("Snap Image");
      snapButton.addActionListener(new ActionListener() {
         @Override
         public void actionPerformed(ActionEvent e) {
            // Multiple images are returned only if there are multiple
            // cameras. We only care about the first image.
            List<Image> images = studio_.live().snap(true);
            Image firstImage = images.get(0);
            showImageInfo(firstImage);
         }
      });
      add(snapButton, "wrap");

      exposureTimeLabel_ = new JLabel("");
      add(exposureTimeLabel_, "split, span, growx");

      // Run an acquisition using the current MDA parameters.
      JButton acquireButton = new JButton("Run Acquisition");
      acquireButton.addActionListener(new ActionListener() {
         @Override
         public void actionPerformed(ActionEvent e) {
            // All GUI event handlers are invoked on the EDT (Event Dispatch
            // Thread). Acquisitions are not allowed to be started from the
            // EDT. Therefore we must make a new thread to run this.
            Thread acqThread = new Thread(new Runnable() {
               @Override
               public void run() {
                  studio_.acquisitions().runAcquisition();
               }
            });
            acqThread.start();
         }
      });
      add(acquireButton, "wrap");

      pack();

      // Registering this class for events means that its event handlers
      // (that is, methods with the @Subscribe annotation) will be invoked when
      // an event occurs. You need to call the right registerForEvents() method
      // to get events; this one is for the application-wide event bus, but
      // there's also Datastore.registerForEvents() for events specific to one
      // Datastore, and DisplayWindow.registerForEvents() for events specific
      // to one image display window.
      studio_.events().registerForEvents(this);
   }

   /**
    * To be invoked, this method must be public and take a single parameter
    * which is the type of the event we care about.
    */
   @Subscribe
   public void onExposureChanged(ExposureChangedEvent event) {
      exposureTimeLabel_.setText(String.format("Camera %s exposure time set to %.2fms",
               event.getCameraName(), event.getNewExposureTime()));
   }

   /**
    * Display some information on the data in the provided image.
    */
   private void showImageInfo(Image image) {
      // See DisplayManager for information on these parameters.
      HistogramData data = studio_.displays().calculateHistogram(
         image, 0, 16, 16, 0, true);
      imageInfoLabel_.setText(String.format(
            "Image size: %dx%d; min: %d, max: %d, mean: %d, std: %.2f",
            image.getWidth(), image.getHeight(), data.getMinVal(),
            data.getMaxVal(), data.getMean(), data.getStdDev()));
   }
}
