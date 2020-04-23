#ifndef HISTORY_HPP
#define HISTORY_HPP

#include "internal/common.hpp"
#include "internal/HistoryCollective.hpp"
#include "AbstractWriter.hpp"

#include <cutehmi/services/Serviceable.hpp>

#include <QTimer>

namespace cutehmi {
namespace dataacquisition {

class CUTEHMI_DATAACQUISITION_API HistoryWriter:
	public AbstractWriter

{
		Q_OBJECT

	public:
		static constexpr int INITIAL_INTERVAL = 100;
		static constexpr int INITIAL_SAMPLES = 100;

		/**
		  Interval [ms] between samples.

		  @assumption{cutehmi::dataacquisition::History-interval_non_negative}
		  Value of @a interval property should be non-negative.
		  */
		Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)

		/**
		  Number of samples per candle to be stored in the database.

		  @assumption{cutehmi::dataacquisition::History-samples_greater_than_zero}
		  Value of @a samples property should be greater than zero.
		  */
		Q_PROPERTY(int samples READ samples WRITE setSamples NOTIFY samplesChanged)

		HistoryWriter(QObject * parent = nullptr);

		int interval() const;

		void setInterval(int interval);

		int samples() const;

		void setSamples(int samples);

		virtual std::unique_ptr<ServiceStatuses> configureStarting(QState * starting) override;

		virtual std::unique_ptr<ServiceStatuses> configureStarted(QState * active, const QState * idling, const QState * yielding) override;

		virtual std::unique_ptr<ServiceStatuses> configureStopping(QState * stopping) override;

		virtual std::unique_ptr<ServiceStatuses> configureBroken(QState * broken) override;

		virtual std::unique_ptr<ServiceStatuses> configureRepairing(QState * repairing) override;

		virtual std::unique_ptr<ServiceStatuses> configureEvacuating(QState * evacuating) override;

		virtual std::unique_ptr<QAbstractTransition> transitionToStarted() const override;

		virtual std::unique_ptr<QAbstractTransition> transitionToStopped() const override;

		virtual std::unique_ptr<QAbstractTransition> transitionToBroken() const override;

		virtual std::unique_ptr<QAbstractTransition> transitionToYielding() const override;

		virtual std::unique_ptr<QAbstractTransition> transitionToIdling() const override;

	signals:
		void intervalChanged();

		void samplesChanged();

	protected slots:
		void sampleValues();

		void insertValues();

	CUTEHMI_PROTECTED_SIGNALS:
		void initialized();

		void samplingTimerStarted();

		void samplingTimerStopped();

		void schemaValidatedTrue();

		void schemaValidatedFalse();

		void insertValuesBegan();

	private slots:
		void onSchemaChanged();

		void initialize();

		void adjustSamplingTimer();

		void startSamplingTimer();

		void stopSamplingTimer();

	private:
		std::unique_ptr<ServiceStatuses> configureStartingOrRepairing(QState * parent);

		void addIntSample(const QString & tagName, int value);

		void addBoolSample(const QString & tagName, bool value);

		void addRealSample(const QString & tagName, double value);

		void clearData();

		template <typename T>
		void addSample(T value, typename internal::HistoryTable<T>::Tuple & tuple);

		struct Members
		{
			internal::HistoryTable<int>::TuplesContainer intTuples;
			internal::HistoryTable<bool>::TuplesContainer boolTuples;
			internal::HistoryTable<double>::TuplesContainer realTuples;
			internal::HistoryCollective dbCollective;
			QTimer samplingTimer;
			int interval;
			int samples;
			int sampleCounter;

			Members():
				interval(INITIAL_INTERVAL),
				samples(INITIAL_SAMPLES),
				sampleCounter(0)
			{
			}
		};

		MPtr<Members> m;
};

}
}

#endif // HISTORY_HPP
