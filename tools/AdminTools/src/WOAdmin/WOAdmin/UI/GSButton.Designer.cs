namespace WOAdmin
{
    partial class GSButton
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.SuspendLayout();
            // 
            // GSButton
            // 
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.ClientSize = new System.Drawing.Size(138, 9);
            this.Font = new System.Drawing.Font("Arial", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.ForeColor = System.Drawing.Color.White;
            this.Name = "GSButton";
            this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.GSButton_MouseUp);
            this.MouseEnter += new System.EventHandler(this.GSButton_MouseEnter);
            this.Click += new System.EventHandler(this.GSButton_Click);
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.GSButton_MouseDown);
            this.MouseLeave += new System.EventHandler(this.GSButton_MouseLeave);
            this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.GSButton_MouseMove);
            this.ResumeLayout(false);

        }

        #endregion
    }
}