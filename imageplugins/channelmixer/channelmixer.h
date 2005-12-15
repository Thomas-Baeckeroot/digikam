/* ============================================================
 * File  : channelmixer.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-26
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef CHANNELMIXER_H
#define CHANNELMIXER_H

// Local includes.

#include "imagetabdialog.h"

class QComboBox;
class QPushButton;
class QCheckBox;
class QHButtonGroup;

class KDoubleNumInput;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageGuideWidget;
class DColor;
}

namespace DigikamChannelMixerImagesPlugin
{

class ChannelMixerDialog : public DigikamImagePlugins::ImageTabDialog
{
    Q_OBJECT

public:

    ChannelMixerDialog(QWidget *parent);
    ~ChannelMixerDialog();

private:

    enum ColorChannelGains 
    {
    RedChannelGains=0,
    GreenChannelGains,
    BlueChannelGains
    };

    enum HistogramScale
    {
    Linear=0,
    Logarithmic
    };

private:
    
    uchar                        *m_destinationPreviewData;
 
    double                        m_redRedGain;
    double                        m_redGreenGain;
    double                        m_redBlueGain;
    double                        m_greenRedGain;
    double                        m_greenGreenGain;
    double                        m_greenBlueGain;
    double                        m_blueRedGain;
    double                        m_blueGreenGain;
    double                        m_blueBlueGain;
    double                        m_blackRedGain;
    double                        m_blackGreenGain;
    double                        m_blackBlueGain;
    
    QComboBox                    *m_channelCB;    
    
    QHButtonGroup                *m_scaleBG;  

    KDoubleNumInput              *m_redGain;
    KDoubleNumInput              *m_greenGain;
    KDoubleNumInput              *m_blueGain;
    
    QPushButton                  *m_resetButton;
    
    QCheckBox                    *m_preserveLuminosity;
    QCheckBox                    *m_monochrome;
    QCheckBox                    *m_overExposureIndicatorBox;
    
    Digikam::ColorGradientWidget *m_hGradient;
    
    Digikam::HistogramWidget     *m_histogramWidget;
    
    Digikam::ImageGuideWidget    *m_previewOriginalWidget;
    Digikam::ImageGuideWidget    *m_previewTargetWidget;
    
private:

    void adjustSliders(void);
    
private slots:

    void slotDefault();
    void slotUser2();
    void slotUser3();
    void slotResetCurrentChannel();
    void slotEffect();
    void slotOk();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotGainsChanged();
    void slotMonochromeActived(bool mono);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);    
};

}  // NameSpace DigikamChannelMixerImagesPlugin

#endif /* CHANNELMIXER_H */
