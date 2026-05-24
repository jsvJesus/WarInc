using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing.Text;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    [Description("GSButton")]
    [ToolboxBitmap(typeof(Button))]
    public partial class GSButton : System.Windows.Forms.Button
    {
        public const int LATERAL_PADDING = 10; // 10 pixels

        private bool _hovering = false;
        private bool hovering
        {
            get { return _hovering; }
            set
            {
                _hovering = value;
                if(_hovering) this.Cursor = Cursors.Hand;
                else this.Cursor = Cursors.Default;
            }
        }

        private bool pressed = false;

        private bool _selected = false;
        public bool StaySelected
        {
            get { return _selected; }
            set
            {
                _selected = value;
                this.Invalidate();
            }
        }

        private Image _imageNormal = null;
        [Browsable(true)]
        public Image ImageNormal
        {
            get { return _imageNormal; }
            set
            {
                _imageNormal = value;
                this.Invalidate();
            }
        }

        private Image _imageHover = null;
        [Browsable(true)]
        public Image ImageHover
        {
            get { return _imageHover; }
            set
            {
                _imageHover = value;
                this.Invalidate();
            }
        }

        private Image _imagePressed = null;
        [Browsable(true)]
        public Image ImagePressed
        {
            get { return _imagePressed; }
            set
            {
                _imagePressed = value;
                this.Invalidate();
            }
        }

        public Color cBtnTextA = Color.Black;
        public Color cBtnTextD = Color.Gray;
        public Color cBtnTextH = Color.Blue;

        // to make sure the control is invalidated(repainted) when the text is changed
        public override string Text
        {
            get
            {
                return base.Text;
            }
            set
            {
                base.Text = value;
                this.Invalidate();
            }
        }

        private bool _gsAutoSize = false;
        [Browsable(true)]
        public bool GSAutoSize
        {
            get
            {
                return _gsAutoSize;
            }
            set
            {
                _gsAutoSize = value;
                if (_gsAutoSize)
                {
                    this.Width = CalcButtonWidth();
                    Invalidate();
                }
            }
        }

        public GSButton()
        {
            Visible = true;
            InitializeComponent();

            //TopLevel = false;
            //FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;

            this.SetStyle(ControlStyles.DoubleBuffer, true);
            this.SetStyle(ControlStyles.SupportsTransparentBackColor, true);
        }

        protected int CalcButtonWidth()
        {
            Graphics g = this.CreateGraphics();
            g.PageUnit = GraphicsUnit.Pixel;
            SizeF sz = g.MeasureString(this.Text, this.Font);
            g.Dispose();

            return (int)sz.Width + LATERAL_PADDING * 2;
        }

        protected override void OnPaintBackground(PaintEventArgs e)
        {
            base.OnPaintBackground(e);
        }

        protected override void OnPaint(System.Windows.Forms.PaintEventArgs e)
        {
            e.Graphics.SmoothingMode = SmoothingMode.None; // AntiAlias;
            e.Graphics.InterpolationMode = InterpolationMode.High;

            e.Graphics.FillRectangle(new SolidBrush(Parent.BackColor), ClientRectangle);

            Image im;

            if (_selected) im = _imagePressed;
            else if (pressed && hovering) im = _imagePressed;
            else if (pressed || hovering) im = _imageHover;
            else im = _imageNormal;
            if(im != null) e.Graphics.DrawImage(im, ClientRectangle);

            Color cA = cBtnTextA;
            Color cH = cBtnTextH;
            Color cD = cBtnTextD;

            StringFormat format1 = new StringFormat(StringFormatFlags.NoClip);
            format1.LineAlignment = StringAlignment.Center;
            format1.Alignment = StringAlignment.Center;
            Color color = cA;
            if (hovering && !pressed) color = cH;
            if (!Enabled) color = cD;

            Rectangle textRect = new Rectangle(ClientRectangle.Left, ClientRectangle.Top, ClientRectangle.Width, ClientRectangle.Height);
            e.Graphics.TextRenderingHint = TextRenderingHint.ClearTypeGridFit;
            e.Graphics.DrawString(this.Text, this.Font, new SolidBrush(color), textRect, format1);
        }

        private void GSButton_MouseEnter(object sender, EventArgs e)
        {
            hovering = true;
            Invalidate();
        }

        private void GSButton_MouseLeave(object sender, EventArgs e)
        {
            hovering = false;
            Invalidate();
        }

        private void GSButton_MouseDown(object sender, MouseEventArgs e)
        {
            pressed = true;
            Capture = true;
            Invalidate();
        }

        private void GSButton_MouseUp(object sender, MouseEventArgs e)
        {
            pressed = false;
            Capture = false;
            Invalidate();
        }

        private void GSButton_Click(object sender, EventArgs e)
        {

        }

        private void GSButton_MouseMove(object sender, MouseEventArgs e)
        {
            bool inside = true;
            if (e.X < 0 || e.X > Width || e.Y < 0 || e.Y > Height) inside = false;

            if (pressed && hovering != inside)
            {
                //System.Diagnostics.Debug.Write("hover changed\n");
                Invalidate();
                hovering = inside;
            }
        }
    }
}